//cpu.c

#include <stdio.h>
#include <stdlib.h>
#include "rombuffer.h"
#include "disassembler.h"
#include "cpu.h"

typedef struct cpu {
    uint8_t registers[16];
    uint16_t I;

    uint8_t delay;
    uint8_t sound_timer;

    uint16_t pc;

    uint8_t sp;
    uint16_t stack[16];

    uint8_t memory[4096];
    const rombuffer_t *rom;
} cpu_t;

static void sys_nnn(cpu_t *cpu, const instruction_t *instruction) {
    //instruction ignored by modern interpreters
}

static void cls(cpu_t *cpu, const instruction_t *instrucdtion) {
    //temporary before screen implementation
    printf("clear the display\n");
}

static void jp_nnn(cpu_t *cpu, const instruction_t *instruction) {
    cpu->pc = instruction->operands[0];
}

void (*executable_instruction_table[])(cpu_t *cpu, const instruction_t *instruction) = {
    jp_nnn
};

cpu_t *cpu_new() {
    cpu_t *cpu = malloc(sizeof(cpu_t));
    if (cpu == NULL) {
        return NULL;
    }

    cpu->I = 0;
    cpu->delay = 0;
    cpu->sound_timer = 0;
    cpu->pc = 0x200;
    cpu->sp = 0;

    for (size_t i = 0; i < 16; i++) {
        cpu->registers[i] = 0;
        cpu->stack[i] = 0;
    }

    for (size_t i = 0; i < 4096; i++) {
        cpu->memory[i] = 0;
    }

    return cpu;
}

int cpu_load(cpu_t *cpu, const rombuffer_t *rom) {
    if (cpu == NULL || rom == NULL) {
        return -1;
    }

    cpu->rom = rom;
    
    return 0;
}

int cpu_reset(cpu_t *cpu) {
    if (cpu == NULL) {
        return -1;
    }

    cpu->rom = NULL;

    return 0;
}

int cpu_run(cpu_t *cpu) {
    if (cpu == NULL) {
        return -1;
    }

    //fetch
    uint16_t opcode = cpu->rom->data[cpu->pc];
    //disassemble
    instruction_t instruction;
    disassembler_disassemble(&instruction, opcode);
    //execute
    void (*f)(cpu_t *cpu, const instruction_t *instruction) = executable_instruction_table[instruction.instruction_info->instruction_type];
    f(cpu, &instruction);

    //increment program counter... what if instruction is JP???
    cpu->pc++;
    
    return 0;
}

void cpu_free(cpu_t *cpu) {
    if (cpu == NULL) {
        return;
    }

    free(cpu);
    return;
}

