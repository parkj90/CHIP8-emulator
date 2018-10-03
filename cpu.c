//cpu.c

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include "rombuffer.h"
#include "disassembler.h"
#include "cpu.h"

typedef struct cpu {
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

static void cpu_execute(cpu_t *cpu);
static uint16_t cpu_fetch_opcode(cpu_t *cpu);

static void cpu_exec_sys_nnn(cpu_t *cpu, const instruction_t *instruction);
static void cpu_exec_cls(cpu_t *cpu, const instruction_t *instruction);
static void cpu_exec_ret(cpu_t *cpu, const instruction_t *instruction);
static void cpu_exec_jp_nnn(cpu_t *cpu, const instruction_t *instruction);
static void cpu_exec_call_nnn(cpu_t *cpu, const instruction_t *instruction);
static void cpu_exec_se_vx_kk(cpu_t *cpu, const instruction_t *instruction);
static void cpu_exec_sne_vx_kk(cpu_t *cpu, const instruction_t *instruction);
static void cpu_exec_se_vx_vy(cpu_t *cpu, const instruction_t *instruction);
static void cpu_exec_ld_vx_kk(cpu_t *cpu, const instruction_t *instruction);
static void cpu_exec_add_vx_kk(cpu_t *cpu, const instruction_t *instruction);
static void cpu_exec_ld_vx_vy(cpu_t *cpu, const instruction_t *instruction);
static void cpu_exec_or_vx_vy(cpu_t *cpu, const instruction_t *instruction);
static void cpu_exec_and_vx_vy(cpu_t *cpu, const instruction_t *instruction);
static void cpu_exec_xor_vx_vy(cpu_t *cpu, const instruction_t *instruction);
static void cpu_exec_add_vx_vy(cpu_t *cpu, const instruction_t *instruction);
static void cpu_exec_sub_vx_vy(cpu_t *cpu, const instruction_t *instruction);
static void cpu_exec_shr_vx_vy(cpu_t *cpu, const instruction_t *instruction);
static void cpu_exec_subn_vx_vy(cpu_t *cpu, const instruction_t *instruction);
static void cpu_exec_shl_vx_vy(cpu_t *cpu, const instruction_t *instruction);
static void cpu_exec_sne_vx_vy(cpu_t *cpu, const instruction_t *instruction);
static void cpu_exec_ld_i_nnn(cpu_t *cpu, const instruction_t *instruction);
static void cpu_exec_jp_v0_nnn(cpu_t *cpu, const instruction_t *instruction);
static void cpu_exec_rnd_vx_kk(cpu_t *cpu, const instruction_t *instruction);
static void cpu_exec_drw_vx_vy_n(cpu_t *cpu, const instruction_t *instruction);
static void cpu_exec_skp_vx(cpu_t *cpu, const instruction_t *instruction);
static void cpu_exec_sknp_vx(cpu_t *cpu, const instruction_t *instruction);
static void cpu_exec_ld_vx_dt(cpu_t *cpu, const instruction_t *instruction);
static void cpu_exec_ld_vx_k(cpu_t *cpu, const instruction_t *instruction);
static void cpu_exec_ld_dt_vx(cpu_t *cpu, const instruction_t *instruction);
static void cpu_exec_ld_st_vx(cpu_t *cpu, const instruction_t *instruction);
static void cpu_exec_add_i_vx(cpu_t *cpu, const instruction_t *instruction);
static void cpu_exec_ld_f_vx(cpu_t *cpu, const instruction_t *instruction);
static void cpu_exec_ld_b_vx(cpu_t *cpu, const instruction_t *instruction);
static void cpu_exec_ld_i_vx(cpu_t *cpu, const instruction_t *instruction);
static void cpu_exec_ld_vx_i(cpu_t *cpu, const instruction_t *instruction);
static void cpu_exec_data(cpu_t *cpu, const instruction_t *instruction);

static void (* const exec_instruction_table[])(cpu_t *cpu, const instruction_t *instruction) = {
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

cpu_t *cpu_new() {
    cpu_t *cpu = malloc(sizeof(cpu_t));
    if (cpu == NULL) {
        return NULL;
    }

    return cpu;
}

int cpu_load(cpu_t *cpu, const rombuffer_t *rom) {
    if (cpu == NULL || rom == NULL) {
        return -1;
    }

    cpu_reset(cpu, rom);
    
    return 0;
}

int cpu_reset(cpu_t *cpu, const rombuffer_t *rom) {
    if (cpu == NULL) {
        return -1;
    }

    memset(cpu, 0, sizeof(cpu_t));

    cpu->pc = 0x200;
    for (size_t i = 0; i < rom->length; i++) {
        cpu->memory[0x200 + (i * 2)] = (uint8_t)((rom->data[i] & 0xFF00) >> 8); 
        cpu->memory[0x200 + (i * 2) + 1] = (uint8_t)(rom->data[i] & 0x00FF); 
    }

    return 0;
}

int cpu_run(cpu_t *cpu) {
    if (cpu == NULL) {
        return -1;
    }
    
    for (int i = 0; i < 100; i++) {
        //debug code
        printf("%5d|  ", i + 1);
        cpu_execute(cpu);
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

static void cpu_execute(cpu_t *cpu) {
    //fetch
    const uint16_t opcode = cpu_fetch_opcode(cpu);
    //decode
    instruction_t instruction;
    disassembler_disassemble(&instruction, opcode);

    //fix me: remove debug print
    char formatted_instruction[20];
    disassembler_format(formatted_instruction, 20, &instruction);
    printf("pc: %x, opcode: %x, instruction: %s\n", cpu->pc, opcode, formatted_instruction);

    //execute
    exec_instruction_table[instruction.instruction_info->instruction_type](cpu, &instruction);
}

static uint16_t cpu_fetch_opcode(cpu_t *cpu) {
    uint16_t opcode = (cpu->memory[cpu->pc] << 8) | (cpu->memory[cpu->pc + 1]);

    return opcode;
}

static void cpu_exec_sys_nnn(cpu_t *cpu, const instruction_t *instruction) {
    //instruction ignored by modern interpreters
}

static void cpu_exec_cls(cpu_t *cpu, const instruction_t *instruction) {
    //temporary before screen implementation
    printf("display cleared\n");
    cpu->pc += 2;
}

//return from a subroutine
static void cpu_exec_ret(cpu_t *cpu, const instruction_t *instruction) {
    cpu->pc = cpu->stack[cpu->sp];
    cpu->sp--;
}

//jump to location nnn
static void cpu_exec_jp_nnn(cpu_t *cpu, const instruction_t *instruction) {
    cpu->pc = instruction->operands[0];
}

//call subroutine at nnn
static void cpu_exec_call_nnn(cpu_t *cpu, const instruction_t *instruction) {
    cpu->sp++;
    cpu->stack[cpu->sp] = cpu->pc;
    cpu->pc = instruction->operands[0];
}

//skip next instruction if Vx == kk
static void cpu_exec_se_vx_kk(cpu_t *cpu, const instruction_t *instruction) {
    if (cpu->registers[instruction->operands[0]] == instruction->operands[1]) {
        cpu->pc += 2;
    }

    cpu->pc += 2;
}

//skip next instruction if Vx != kk
static void cpu_exec_sne_vx_kk(cpu_t *cpu, const instruction_t *instruction) {
    if (cpu->registers[instruction->operands[0]] != instruction->operands[1]) {
        cpu->pc += 2;
    }

    cpu->pc += 2;
}

//skip next instruction if Vx == Vy
static void cpu_exec_se_vx_vy(cpu_t *cpu, const instruction_t *instruction) {
    if (cpu->registers[instruction->operands[0]] == cpu->registers[instruction->operands[1]]) {
        cpu->pc += 2;
    }

    cpu->pc += 2;
}

//set Vx = kk
static void cpu_exec_ld_vx_kk(cpu_t *cpu, const instruction_t *instruction) {
    cpu->registers[instruction->operands[0]] = instruction->operands[1];

    cpu->pc += 2;
}

//sets Vx = Vx + kk
static void cpu_exec_add_vx_kk(cpu_t *cpu, const instruction_t *instruction) {
    cpu->registers[instruction->operands[0]] += instruction->operands[1];

    cpu->pc += 2;
}

//set Vx = Vy
static void cpu_exec_ld_vx_vy(cpu_t *cpu, const instruction_t *instruction) {
    cpu->registers[instruction->operands[0]] = cpu->registers[instruction->operands[1]];

    cpu->pc += 2;
}

//set Vx = Vx OR Vy
static void cpu_exec_or_vx_vy(cpu_t *cpu, const instruction_t *instruction) {
    cpu->registers[instruction->operands[0]] |= cpu->registers[instruction->operands[1]];

    cpu->pc += 2;
}

//set Vx = Vx AND Vy
static void cpu_exec_and_vx_vy(cpu_t *cpu, const instruction_t *instruction) {
    cpu->registers[instruction->operands[0]] &= cpu->registers[instruction->operands[1]];

    cpu->pc += 2;
}

//set Vx = Vx XOR Vy
static void cpu_exec_xor_vx_vy(cpu_t *cpu, const instruction_t *instruction) {
    cpu->registers[instruction->operands[0]] ^= cpu->registers[instruction->operands[1]];

    cpu->pc += 2;
}

//set Vx = Vx + Vy, set VF = carry
static void cpu_exec_add_vx_vy(cpu_t *cpu, const instruction_t *instruction) {
    if (cpu->registers[instruction->operands[0]] + cpu->registers[instruction->operands[1]] > 0xFF) {
        cpu->VF = true;
    } else {
        cpu->VF = false;
    }

    cpu->registers[instruction->operands[0]] += cpu->registers[instruction->operands[1]];

    cpu->pc += 2;
}

//set Vx = Vx - Vy, set VF = NOT borrow
static void cpu_exec_sub_vx_vy(cpu_t *cpu, const instruction_t *instruction) {
    if (cpu->registers[instruction->operands[0]] > cpu->registers[instruction->operands[1]]) {
        cpu->VF = true;
    } else {
        cpu->VF = false;
    }

    cpu->registers[instruction->operands[0]] -= cpu->registers[instruction->operands[1]];

    cpu->pc += 2;
}

//set Vx = Vx + SHR 1
static void cpu_exec_shr_vx_vy(cpu_t *cpu, const instruction_t *instruction) {
    if ((cpu->registers[instruction->operands[0]] & 0x01) == 0x01) {
        cpu->VF = true;
    } else {
        cpu->VF = false;
    }

    cpu->registers[instruction->operands[0]] >>= 1;

    cpu->pc += 2;
}

//set Vx = Vy - Vx, set VF = NOT borrow
static void cpu_exec_subn_vx_vy(cpu_t *cpu, const instruction_t *instruction) {
    if (cpu->registers[instruction->operands[1]] > cpu->registers[instruction->operands[0]]) {
        cpu->VF = true;
    } else {
        cpu->VF = false;
    }

    cpu->registers[instruction->operands[0]] = cpu->registers[instruction->operands[1]] - cpu->registers[instruction->operands[0]];

    cpu->pc += 2;
}

//set Vx = Vx SHL 1
static void cpu_exec_shl_vx_vy(cpu_t *cpu, const instruction_t *instruction) {
    if ((cpu->registers[instruction->operands[0]] & 0x80) == 0x80) {
        cpu->VF = true;
    } else {
        cpu->VF = false;
    }

    cpu->registers[instruction->operands[0]] <<= 1;

    cpu->pc += 2;
}

//skip next instruction if Vx != Vy
static void cpu_exec_sne_vx_vy(cpu_t *cpu, const instruction_t *instruction) {
    if (cpu->registers[instruction->operands[0]] != cpu->registers[instruction->operands[1]]) {
        cpu->pc += 2;
    }

    cpu->pc += 2;
}

//set I = nnn
static void cpu_exec_ld_i_nnn(cpu_t *cpu, const instruction_t *instruction) {
    cpu->I = instruction->operands[1];

    cpu->pc += 2;
}

//jump to location nnn + V0
static void cpu_exec_jp_v0_nnn(cpu_t *cpu, const instruction_t *instruction) {
    cpu->pc = instruction->operands[1] + cpu->registers[0];
}

//set Vx = random byte and kk
static void cpu_exec_rnd_vx_kk(cpu_t *cpu, const instruction_t *instruction) {
    cpu->registers[instruction->operands[0]] = rand() % 256 + instruction->operands[1];

    cpu->pc += 2;
}

//display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision
static void cpu_exec_drw_vx_vy_n(cpu_t *cpu, const instruction_t *instruction) {
    printf("drawing sprites... \n");
    /*
    for (uint16_t i = 0; i < instruction->operands[2]; i++) {
        uint8_t sprite = cpu->memory[cpu->I + i];
        uint8_t coord_x = cpu->registers[instruction->operands[0]];
        uint8_t coord_y = cpu->registers[instruction->operands[1]];

        //fix me: VF collision detection (pixel erased), wrap to opposite side
        printf("sprite: %x displayed at %x, %x\n", sprite, coord_x, coord_y);
    }
    */

    cpu->pc += 2;
}

//skip next instruction if key wth the value of Vx is pressed
static void cpu_exec_skp_vx(cpu_t *cpu, const instruction_t *instruction) {
    //fix me: check keyboard input
    bool key = false;// = keyboard at: cpu->registers[instruction->operands[0]];
    if (key) {
        cpu->pc += 2;
    }

    cpu->pc += 2;
}

//skip next instruction if key with the value of Vx is not pressed
static void cpu_exec_sknp_vx(cpu_t *cpu, const instruction_t *instruction) {
    //fix me: check keyboard input
    bool key = false; // = keyboard at: cpu->registers[instruction->operands[0]];
    if (!key) {
        cpu->pc += 2;
    }

    cpu->pc += 2;
}

//set Vx = delay timer value
static void cpu_exec_ld_vx_dt(cpu_t *cpu, const instruction_t *instruction) {
    cpu->registers[instruction->operands[0]] = cpu->DT;

    cpu->pc += 2;
}

//wait for a key press, store the value of the key in Vx
static void cpu_exec_ld_vx_k(cpu_t *cpu, const instruction_t *instruction) {
    //loop until key is pressed
    uint8_t keystroke = 0x00;

    cpu->registers[instruction->operands[0]] = keystroke;

    cpu->pc += 2;
}

//set delay timer = Vx
static void cpu_exec_ld_dt_vx(cpu_t *cpu, const instruction_t *instruction) {
    cpu->DT = cpu->registers[instruction->operands[1]];

    cpu->pc += 2;
}

//set sound timer = Vx
static void cpu_exec_ld_st_vx(cpu_t *cpu, const instruction_t *instruction) {
    cpu->ST = cpu->registers[instruction->operands[1]];

    cpu->pc += 2;
}

//set I = I + Vx
static void cpu_exec_add_i_vx(cpu_t *cpu, const instruction_t *instruction) {
    cpu->I += cpu->registers[instruction->operands[1]];

    cpu->pc += 2;
}

//set I = location of sprite for digit Vx
static void cpu_exec_ld_f_vx(cpu_t *cpu, const instruction_t *instruction) {
    cpu->I = cpu->memory[cpu->registers[instruction->operands[1]]];

    cpu->pc += 2;
}

//store BCD representaion of Vx in memory locations I, I+1, and I+2
static void cpu_exec_ld_b_vx(cpu_t *cpu, const instruction_t *instruction) {
    uint8_t decimal = cpu->registers[instruction->operands[1]];

    for (int i = 2; i >= 0; i--) {
        cpu->memory[cpu->I + i] = decimal % 10;
        decimal /= 10;
    }

    cpu->pc += 2;
}

//store registers V0 through Vx in memory starting at location I
static void cpu_exec_ld_i_vx(cpu_t *cpu, const instruction_t *instruction) {
    for (uint16_t i = 0; i <= instruction->operands[1]; i++) {
        cpu->memory[cpu->I + i] = cpu->registers[i];
    }

    cpu->pc += 2;
}

//read registers V0 through Vx from memory starting at location I
static void cpu_exec_ld_vx_i(cpu_t *cpu, const instruction_t *instruction) {
    for (uint16_t i = 0; i <= instruction->operands[0]; i++) {
         cpu->registers[i] = cpu->memory[cpu->I + i];
    }

    cpu->pc += 2;
}

static void cpu_exec_data(cpu_t *cpu, const instruction_t *instruction) {
    //fix me: use exit()?
    //abort();
}
