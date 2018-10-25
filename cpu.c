//cpu.c

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include "rombuffer.h"
#include "disassembler.h"
#include "cpu.h"

#include <unistd.h>

#define SPRITE_WIDTH 8          //8 bit sprite width
#define FONT_SPRITE_HEIGHT 5

typedef struct cpu {
    const cpu_io_interface_t *cpu_io_interface;

    uint8_t registers[16];
    uint16_t I;

    bool VF;

    uint8_t DT;
    uint8_t ST;

    uint16_t pc;

    uint8_t sp;
    uint16_t stack[16];

    uint8_t memory[4096];
} cpu_t;

static const uint8_t font_library[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0,       // 0
    0x20, 0x60, 0x20, 0x20, 0x70,       // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0,       // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0,       // 3
    0x90, 0x90, 0xF0, 0x10, 0x10,       // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0,       // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0,       // 6
    0xF0, 0x10, 0x20, 0x40, 0x40,       // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0,       // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0,       // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90,       // a
    0xE0, 0x90, 0xE0, 0x90, 0xE0,       // b
    0xF0, 0x80, 0x80, 0x80, 0xF0,       // c
    0xE0, 0x90, 0x90, 0x90, 0xE0,       // d
    0xF0, 0x80, 0xF0, 0x80, 0xF0,       // e
    0xF0, 0x80, 0xF0, 0x80, 0x80,       // f
};

static const size_t font_library_size = sizeof(font_library) / sizeof(uint8_t);

static int cpu_execute(cpu_t *cpu);
static uint16_t cpu_fetch_opcode(cpu_t *cpu);

enum cpu_error_code {
    CPU_ERROR_QUIT             =  1,    //program terminated with keyboard input
    CPU_ERROR_SUCCESS          =  0,    //success
    CPU_ERROR_NULL_PNTR        = -1,    //NULL pointer returned
    CPU_ERROR_8BIT_OOB         = -2,    //value exceeds 0x0F
    CPU_ERROR_PC_OOB           = -3,    //pc set to memory address < 0x2
    CPU_ERROR_WRITE_OOB        = -4,    //attempted write to memory addr
    CPU_ERROR_STACK_UNDERFLOW  = -5,    //stack underflow
    CPU_ERROR_STACK_OVERFLOW   = -6,    //stack overflow
    CPU_ERROR_DATA_EXEC        = -7,    //data opcode ran as executable
    CPU_ERROR_UNSUPORTED       = -8     //unsupported opcode ran as executable
};

static int cpu_exec_sys_nnn(cpu_t *cpu, const instruction_t *instruction);
static int cpu_exec_cls(cpu_t *cpu, const instruction_t *instruction);
static int cpu_exec_ret(cpu_t *cpu, const instruction_t *instruction);
static int cpu_exec_jp_nnn(cpu_t *cpu, const instruction_t *instruction);
static int cpu_exec_call_nnn(cpu_t *cpu, const instruction_t *instruction);
static int cpu_exec_se_vx_kk(cpu_t *cpu, const instruction_t *instruction);
static int cpu_exec_sne_vx_kk(cpu_t *cpu, const instruction_t *instruction);
static int cpu_exec_se_vx_vy(cpu_t *cpu, const instruction_t *instruction);
static int cpu_exec_ld_vx_kk(cpu_t *cpu, const instruction_t *instruction);
static int cpu_exec_add_vx_kk(cpu_t *cpu, const instruction_t *instruction);
static int cpu_exec_ld_vx_vy(cpu_t *cpu, const instruction_t *instruction);
static int cpu_exec_or_vx_vy(cpu_t *cpu, const instruction_t *instruction);
static int cpu_exec_and_vx_vy(cpu_t *cpu, const instruction_t *instruction);
static int cpu_exec_xor_vx_vy(cpu_t *cpu, const instruction_t *instruction);
static int cpu_exec_add_vx_vy(cpu_t *cpu, const instruction_t *instruction);
static int cpu_exec_sub_vx_vy(cpu_t *cpu, const instruction_t *instruction);
static int cpu_exec_shr_vx_vy(cpu_t *cpu, const instruction_t *instruction);
static int cpu_exec_subn_vx_vy(cpu_t *cpu, const instruction_t *instruction);
static int cpu_exec_shl_vx_vy(cpu_t *cpu, const instruction_t *instruction);
static int cpu_exec_sne_vx_vy(cpu_t *cpu, const instruction_t *instruction);
static int cpu_exec_ld_i_nnn(cpu_t *cpu, const instruction_t *instruction);
static int cpu_exec_jp_v0_nnn(cpu_t *cpu, const instruction_t *instruction);
static int cpu_exec_rnd_vx_kk(cpu_t *cpu, const instruction_t *instruction);
static int cpu_exec_drw_vx_vy_n(cpu_t *cpu, const instruction_t *instruction);
static int cpu_exec_skp_vx(cpu_t *cpu, const instruction_t *instruction);
static int cpu_exec_sknp_vx(cpu_t *cpu, const instruction_t *instruction);
static int cpu_exec_ld_vx_dt(cpu_t *cpu, const instruction_t *instruction);
static int cpu_exec_ld_vx_k(cpu_t *cpu, const instruction_t *instruction);
static int cpu_exec_ld_dt_vx(cpu_t *cpu, const instruction_t *instruction);
static int cpu_exec_ld_st_vx(cpu_t *cpu, const instruction_t *instruction);
static int cpu_exec_add_i_vx(cpu_t *cpu, const instruction_t *instruction);
static int cpu_exec_ld_f_vx(cpu_t *cpu, const instruction_t *instruction);
static int cpu_exec_ld_b_vx(cpu_t *cpu, const instruction_t *instruction);
static int cpu_exec_ld_i_vx(cpu_t *cpu, const instruction_t *instruction);
static int cpu_exec_ld_vx_i(cpu_t *cpu, const instruction_t *instruction);
static int cpu_exec_data(cpu_t *cpu, const instruction_t *instruction);

static int (* const exec_instruction_table[])(cpu_t *cpu, const instruction_t *instruction) = {
    [INSTRUCTION_SYS_NNN]     = cpu_exec_sys_nnn,
    [INSTRUCTION_CLS]         = cpu_exec_cls,
    [INSTRUCTION_RET]         = cpu_exec_ret,
    [INSTRUCTION_JP_NNN]      = cpu_exec_jp_nnn,
    [INSTRUCTION_CALL_NNN]    = cpu_exec_call_nnn,
    [INSTRUCTION_SE_VX_KK]    = cpu_exec_se_vx_kk,
    [INSTRUCTION_SNE_VX_KK]   = cpu_exec_sne_vx_kk,
    [INSTRUCTION_SE_VX_VY]    = cpu_exec_se_vx_vy,
    [INSTRUCTION_LD_VX_KK]    = cpu_exec_ld_vx_kk,
    [INSTRUCTION_ADD_VX_KK]   = cpu_exec_add_vx_kk,
    [INSTRUCTION_LD_VX_VY]    = cpu_exec_ld_vx_vy,
    [INSTRUCTION_OR_VX_VY]    = cpu_exec_or_vx_vy,
    [INSTRUCTION_AND_VX_VY]   = cpu_exec_and_vx_vy,
    [INSTRUCTION_XOR_VX_VY]   = cpu_exec_xor_vx_vy,
    [INSTRUCTION_ADD_VX_VY]   = cpu_exec_add_vx_vy,
    [INSTRUCTION_SUB_VX_VY]   = cpu_exec_sub_vx_vy,
    [INSTRUCTION_SHR_VX_VY]   = cpu_exec_shr_vx_vy,
    [INSTRUCTION_SUBN_VX_VY]  = cpu_exec_subn_vx_vy,
    [INSTRUCTION_SHL_VX_VY]   = cpu_exec_shl_vx_vy,
    [INSTRUCTION_SNE_VX_VY]   = cpu_exec_sne_vx_vy,
    [INSTRUCTION_LD_I_NNN]    = cpu_exec_ld_i_nnn,
    [INSTRUCTION_JP_V0_NNN]   = cpu_exec_jp_v0_nnn,
    [INSTRUCTION_RND_VX_KK]   = cpu_exec_rnd_vx_kk,
    [INSTRUCTION_DRW_VX_VY_N] = cpu_exec_drw_vx_vy_n,
    [INSTRUCTION_SKP_VX]      = cpu_exec_skp_vx,
    [INSTRUCTION_SKNP_VX]     = cpu_exec_sknp_vx,
    [INSTRUCTION_LD_VX_DT]    = cpu_exec_ld_vx_dt,
    [INSTRUCTION_LD_VX_K]     = cpu_exec_ld_vx_k,
    [INSTRUCTION_LD_DT_VX]    = cpu_exec_ld_dt_vx,
    [INSTRUCTION_LD_ST_VX]    = cpu_exec_ld_st_vx,
    [INSTRUCTION_ADD_I_VX]    = cpu_exec_add_i_vx,
    [INSTRUCTION_LD_F_VX]     = cpu_exec_ld_f_vx,
    [INSTRUCTION_LD_B_VX]     = cpu_exec_ld_b_vx,
    [INSTRUCTION_LD_I_VX]     = cpu_exec_ld_i_vx,
    [INSTRUCTION_LD_VX_I]     = cpu_exec_ld_vx_i,
    [INSTRUCTION_DATA]        = cpu_exec_data
};

cpu_t *cpu_new(const cpu_io_interface_t *cpu_io_interface) {
    cpu_t *cpu = malloc(sizeof(cpu_t));
    if (cpu == NULL) {
        return NULL;
    }

    cpu->cpu_io_interface = cpu_io_interface;

    return cpu;
}

int cpu_load(cpu_t *cpu, const rombuffer_t *rom) {
    if (cpu == NULL || rom == NULL) {
        return CPU_ERROR_NULL_PNTR;
    }

    cpu_reset(cpu, rom);
    
    return 0;
}

int cpu_reset(cpu_t *cpu, const rombuffer_t *rom) {
    if (cpu == NULL) {
        return CPU_ERROR_NULL_PNTR;
    }

    memset(cpu->registers, 0, sizeof(cpu->registers));
    cpu->I = 0;
    cpu->VF = false;
    cpu->DT = 0;
    cpu->ST = 0;
    cpu->pc = 0;
    cpu->sp = 0;
    memset(cpu->stack, 0, sizeof(cpu->stack));
    memset(cpu->memory, 0, sizeof(cpu->memory));

    memcpy(cpu->memory, font_library, font_library_size);

    cpu->pc = 0x200;
    for (size_t i = 0; i < rom->length; i++) {
        cpu->memory[0x200 + (i * 2)] = (uint8_t)((rom->data[i] & 0xFF00) >> 8); 
        cpu->memory[0x200 + (i * 2) + 1] = (uint8_t)(rom->data[i] & 0x00FF); 
    }

    return 0;
}

int cpu_run(cpu_t *cpu) {
    if (cpu == NULL) {
        return CPU_ERROR_NULL_PNTR;
    }
    
    for (int i = 0; i < 100000; i++) {
        int error_code;
        if ((error_code = cpu_execute(cpu))) {
            return error_code;
        }
    }

    return 0;
}

void cpu_free(cpu_t *cpu) {
    if (cpu == NULL) {
        return;
    }

    free(cpu);
    return;
}

static int cpu_execute(cpu_t *cpu) {
    //fetch
    const uint16_t opcode = cpu_fetch_opcode(cpu);
    //decode
    instruction_t instruction;
    disassembler_disassemble(&instruction, opcode);

    //execute
    int error_code;
    if ((error_code = exec_instruction_table[instruction.instruction_info->instruction_type](cpu, &instruction))) {
        return error_code;
    }

    return 0;
}

static uint16_t cpu_fetch_opcode(cpu_t *cpu) {
    uint16_t opcode = (cpu->memory[cpu->pc] << 8) | (cpu->memory[cpu->pc + 1]);

    return opcode;
}

static int cpu_exec_sys_nnn(cpu_t *cpu, const instruction_t *instruction) {
    //instruction ignored by modern interpreters
    return 0;
}

static int cpu_exec_cls(cpu_t *cpu, const instruction_t *instruction) {
    for (int i = 0; i < DISPLAY_WIDTH; i++) {
        for (int j = 0; j < DISPLAY_HEIGHT; j++) {
            cpu->cpu_io_interface->draw_pixel(i, j, false);
        }
    }

    cpu->pc += 2;

    return 0;
}

//return from a subroutine
static int cpu_exec_ret(cpu_t *cpu, const instruction_t *instruction) {
    if (cpu->stack[cpu->sp] < 0x200) {
        return CPU_ERROR_PC_OOB;
    }

    cpu->pc = cpu->stack[cpu->sp];

    if (cpu->sp == 0) {
        return CPU_ERROR_STACK_UNDERFLOW;
    }
    cpu->sp--;

    return 0;
}

//jump to location nnn
static int cpu_exec_jp_nnn(cpu_t *cpu, const instruction_t *instruction) {
    if (instruction->operands[0] < 0x200) {
        return CPU_ERROR_PC_OOB;
    }

    cpu->pc = instruction->operands[0];

    return 0;
}

//call subroutine at nnn
static int cpu_exec_call_nnn(cpu_t *cpu, const instruction_t *instruction) {
    if (cpu->sp == 15) {
        return CPU_ERROR_STACK_OVERFLOW;
    }

    cpu->sp++;
    cpu->stack[cpu->sp] = cpu->pc;

    if (instruction->operands[0] < 0x200) {
        return CPU_ERROR_PC_OOB;
    }

    cpu->pc = instruction->operands[0];

    return 0;
}

//skip next instruction if Vx == kk
static int cpu_exec_se_vx_kk(cpu_t *cpu, const instruction_t *instruction) {
    if (cpu->registers[instruction->operands[0]] == instruction->operands[1]) {
        cpu->pc += 2;
    }

    cpu->pc += 2;
    
    return 0;
}

//skip next instruction if Vx != kk
static int cpu_exec_sne_vx_kk(cpu_t *cpu, const instruction_t *instruction) {
    if (cpu->registers[instruction->operands[0]] != instruction->operands[1]) {
        cpu->pc += 2;
    }

    cpu->pc += 2;

    return 0;
}

//skip next instruction if Vx == Vy
static int cpu_exec_se_vx_vy(cpu_t *cpu, const instruction_t *instruction) {
    if (cpu->registers[instruction->operands[0]] == cpu->registers[instruction->operands[1]]) {
        cpu->pc += 2;
    }

    cpu->pc += 2;

    return 0;
}

//set Vx = kk
static int cpu_exec_ld_vx_kk(cpu_t *cpu, const instruction_t *instruction) {
    cpu->registers[instruction->operands[0]] = instruction->operands[1];

    cpu->pc += 2;
    
    return 0;
}

//sets Vx = Vx + kk
static int cpu_exec_add_vx_kk(cpu_t *cpu, const instruction_t *instruction) {
    cpu->registers[instruction->operands[0]] += instruction->operands[1];

    cpu->pc += 2;

    return 0;
}

//set Vx = Vy
static int cpu_exec_ld_vx_vy(cpu_t *cpu, const instruction_t *instruction) {
    cpu->registers[instruction->operands[0]] = cpu->registers[instruction->operands[1]];

    cpu->pc += 2;

    return 0;
}

//set Vx = Vx OR Vy
static int cpu_exec_or_vx_vy(cpu_t *cpu, const instruction_t *instruction) {
    cpu->registers[instruction->operands[0]] |= cpu->registers[instruction->operands[1]];

    cpu->pc += 2;

    return 0;
}

//set Vx = Vx AND Vy
static int cpu_exec_and_vx_vy(cpu_t *cpu, const instruction_t *instruction) {
    cpu->registers[instruction->operands[0]] &= cpu->registers[instruction->operands[1]];

    cpu->pc += 2;

    return 0;
}

//set Vx = Vx XOR Vy
static int cpu_exec_xor_vx_vy(cpu_t *cpu, const instruction_t *instruction) {
    cpu->registers[instruction->operands[0]] ^= cpu->registers[instruction->operands[1]];

    cpu->pc += 2;

    return 0;
}

//set Vx = Vx + Vy, set VF = carry
static int cpu_exec_add_vx_vy(cpu_t *cpu, const instruction_t *instruction) {
    if (cpu->registers[instruction->operands[0]] + cpu->registers[instruction->operands[1]] > 0xFF) {
        cpu->VF = true;
    } else {
        cpu->VF = false;
    }

    cpu->registers[instruction->operands[0]] += cpu->registers[instruction->operands[1]];

    cpu->pc += 2;

    return 0;
}

//set Vx = Vx - Vy, set VF = NOT borrow
static int cpu_exec_sub_vx_vy(cpu_t *cpu, const instruction_t *instruction) {
    if (cpu->registers[instruction->operands[0]] > cpu->registers[instruction->operands[1]]) {
        cpu->VF = true;
    } else {
        cpu->VF = false;
    }

    cpu->registers[instruction->operands[0]] -= cpu->registers[instruction->operands[1]];

    cpu->pc += 2;

    return 0;
}

//set Vx = Vx + SHR 1
static int cpu_exec_shr_vx_vy(cpu_t *cpu, const instruction_t *instruction) {
    cpu->VF = cpu->registers[instruction->operands[0]] & 0x01;

    cpu->registers[instruction->operands[0]] >>= 1;

    cpu->pc += 2;

    return 0;
}

//set Vx = Vy - Vx, set VF = NOT borrow
static int cpu_exec_subn_vx_vy(cpu_t *cpu, const instruction_t *instruction) {
    if (cpu->registers[instruction->operands[1]] > cpu->registers[instruction->operands[0]]) {
        cpu->VF = true;
    } else {
        cpu->VF = false;
    }

    cpu->registers[instruction->operands[0]] = cpu->registers[instruction->operands[1]] - cpu->registers[instruction->operands[0]];

    cpu->pc += 2;

    return 0;
}

//set Vx = Vx SHL 1
static int cpu_exec_shl_vx_vy(cpu_t *cpu, const instruction_t *instruction) {
    cpu->VF = cpu->registers[instruction->operands[0]] & 0x80;

    cpu->registers[instruction->operands[0]] <<= 1;

    cpu->pc += 2;

    return 0;
}

//skip next instruction if Vx != Vy
static int cpu_exec_sne_vx_vy(cpu_t *cpu, const instruction_t *instruction) {
    if (cpu->registers[instruction->operands[0]] != cpu->registers[instruction->operands[1]]) {
        cpu->pc += 2;
    }

    cpu->pc += 2;

    return 0;
}

//set I = nnn
static int cpu_exec_ld_i_nnn(cpu_t *cpu, const instruction_t *instruction) {
    cpu->I = instruction->operands[1];

    cpu->pc += 2;

    return 0;
}

//jump to location nnn + V0
static int cpu_exec_jp_v0_nnn(cpu_t *cpu, const instruction_t *instruction) {
    if (instruction->operands[1] + cpu->registers[0] < 0x200) {
        return CPU_ERROR_PC_OOB;
    }

    cpu->pc = instruction->operands[1] + cpu->registers[0];

    return 0;
}

//set Vx = random byte and kk
static int cpu_exec_rnd_vx_kk(cpu_t *cpu, const instruction_t *instruction) {
    cpu->registers[instruction->operands[0]] = (rand() & 0xFF) & instruction->operands[1];

    cpu->pc += 2;

    return 0;
}

//display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision
static int cpu_exec_drw_vx_vy_n(cpu_t *cpu, const instruction_t *instruction) {
    //sprite coordinates
    uint8_t x = cpu->registers[instruction->operands[0]];
    uint8_t y = cpu->registers[instruction->operands[1]];

    cpu->VF = false;

    //for each byte of n-byte sprite
    for (uint16_t i = 0; i < instruction->operands[2]; i++) {
        uint8_t display_state = 0x00;
        for (uint8_t j = 0; j < SPRITE_WIDTH; j++) {
            display_state <<= 1;
            if (cpu->cpu_io_interface->get_pixel((x + j) % DISPLAY_WIDTH, (y + i) % DISPLAY_HEIGHT)) {
                display_state |= 1;
            }
        }

        if (display_state & cpu->memory[cpu->I + i]) {
            cpu->VF = true;
        }

        //XOR onto existing screen
        display_state ^= cpu->memory[cpu->I + i];
        for (uint8_t j = 0; j < SPRITE_WIDTH; j++) {
            if (display_state & (0x01 << (7 - j))) {
                cpu->cpu_io_interface->draw_pixel((x + j) % DISPLAY_WIDTH, (y + i) % DISPLAY_HEIGHT, true);
            } else {
                cpu->cpu_io_interface->draw_pixel((x + j) % DISPLAY_WIDTH, (y + i) % DISPLAY_HEIGHT, false);
            }
        }
    }

    cpu->pc += 2;

    return 0;
}

//skip next instruction if key wth the value of Vx is pressed
static int cpu_exec_skp_vx(cpu_t *cpu, const instruction_t *instruction) {
    uint8_t key_value = cpu->registers[instruction->operands[0]];
    if (key_value > 0x0F) {
        return CPU_ERROR_8BIT_OOB;
    }

    uint16_t bitmask = 1 << key_value;

    uint32_t key_input = cpu->cpu_io_interface->get_keyboard();
    
    if (key_input > 0xFFFF) {
        return CPU_ERROR_QUIT;
    }

    if (key_input & bitmask) {
        cpu->pc += 2;
    }

    cpu->pc += 2;

    return 0;
}

//skip next instruction if key with the value of Vx is not pressed
static int cpu_exec_sknp_vx(cpu_t *cpu, const instruction_t *instruction) {
    uint8_t key_value = cpu->registers[instruction->operands[0]];
    if (key_value > 0x0F) {
        return CPU_ERROR_8BIT_OOB;
    }

    uint16_t bitmask = 1 << key_value;

    uint32_t keyboard_state = cpu->cpu_io_interface->get_keyboard();

    if (keyboard_state > 0xFFFF) {
        return CPU_ERROR_QUIT;
    }

    if (!(keyboard_state & bitmask)) {
        cpu->pc += 2;
    }

    cpu->pc += 2;

    return 0;
}

//set Vx = delay timer value
static int cpu_exec_ld_vx_dt(cpu_t *cpu, const instruction_t *instruction) {
    //delay timer funcionality has not been added yet
    return -8;

    /*
    cpu->registers[instruction->operands[0]] = cpu->DT;

    cpu->pc += 2;
    
    return 0;
    */
}

//wait for a key press, store the value of the key in Vx
static int cpu_exec_ld_vx_k(cpu_t *cpu, const instruction_t *instruction) {
    uint8_t key = cpu->cpu_io_interface->wait_keypress();

    if (key > 0x0F) {
        return CPU_ERROR_QUIT;
    }

    cpu->registers[instruction->operands[0]] = key;

    cpu->pc += 2;

    return 0;
}

//set delay timer = Vx
static int cpu_exec_ld_dt_vx(cpu_t *cpu, const instruction_t *instruction) {
    //delay timer funcionality has not been added yet
    return -8;

    /*
    cpu->DT = cpu->registers[instruction->operands[1]];

    cpu->pc += 2;

    return 0;
    */
}

//set sound timer = Vx
static int cpu_exec_ld_st_vx(cpu_t *cpu, const instruction_t *instruction) {
    //sound timer funcionality has not been added yet
    return -8;

    /*
    cpu->ST = cpu->registers[instruction->operands[1]];

    cpu->pc += 2;

    return 0;
    */
}

//set I = I + Vx
static int cpu_exec_add_i_vx(cpu_t *cpu, const instruction_t *instruction) {
    if (cpu->I < 0x200) {
        return CPU_ERROR_WRITE_OOB;
    }

    cpu->I += cpu->registers[instruction->operands[1]];
    cpu->pc += 2;
    
    return 0;
}

//set I = location of sprite for digit Vx
static int cpu_exec_ld_f_vx(cpu_t *cpu, const instruction_t *instruction) {
    if (cpu->registers[instruction->operands[1]] > 0x0F) {
        return CPU_ERROR_8BIT_OOB;
    }

    cpu->I = cpu->registers[instruction->operands[1]] * FONT_SPRITE_HEIGHT;

    cpu->pc += 2;

    return 0;
}

//store BCD representaion of Vx in memory locations I, I+1, and I+2
static int cpu_exec_ld_b_vx(cpu_t *cpu, const instruction_t *instruction) {
    if (cpu->I < 0x200) {
        return CPU_ERROR_WRITE_OOB;
    }

    uint8_t decimal = cpu->registers[instruction->operands[1]];

    cpu->memory[cpu->I + 2] = decimal % 10;
    decimal /= 10;
    cpu->memory[cpu->I + 1] = decimal % 10;
    decimal /= 10;
    cpu->memory[cpu->I] = decimal % 10;

    cpu->pc += 2;

    return 0;
}

//store registers V0 through Vx in memory starting at location I
static int cpu_exec_ld_i_vx(cpu_t *cpu, const instruction_t *instruction) {
    if (cpu->I < 0x200) {
        return CPU_ERROR_WRITE_OOB;
    }

    for (uint16_t i = 0; i <= instruction->operands[1]; i++) {
        cpu->memory[cpu->I + i] = cpu->registers[i];
    }

    cpu->pc += 2;

    return 0;
}

//read registers V0 through Vx from memory starting at location I
static int cpu_exec_ld_vx_i(cpu_t *cpu, const instruction_t *instruction) {
    for (uint16_t i = 0; i <= instruction->operands[0]; i++) {
         cpu->registers[i] = cpu->memory[cpu->I + i];
    }

    cpu->pc += 2;
    
    return 0;
}

static int cpu_exec_data(cpu_t *cpu, const instruction_t *instruction) {
    return CPU_ERROR_DATA_EXEC;
}
