//cpu.c

#include <stdlib.h>
#include "rombuffer.h"
#include "disassembler.h"
#include "cpu.h"

typedef struct cpu {
    uint8_t *registers;

    uint16_t program_counter;

    uint8_t stack_ptr;
    uint16_t *stack;

    rombuffer_t *rom;
} cpu_t;

cpu_t *cpu_new() {
    cpu_t *cpu = malloc(sizeof(cpu_t));
    if (cpu == NULL) {
        return NULL;
    }

    cpu->registers = malloc(16 * sizeof(uint8_t));
    cpu->program_counter = 0;
    cpu->stack_ptr = 0;
    cpu->stack = malloc(16 * sizeof(uint16_t));
}

int cpu_load(cpu_t *cpu, rombuffer_t *rom) {
    if (cpu == NULL || rom == NULL) {
        return -1;
    }

    cpu->rom = rom;
}

int cpu_run(cpu_t *cpu) {
    //fetch
    uint16_t opcode = cpu->rom->data[cpu->program_counter];
    //disassemble
    instruction_t instruction;
    disassembler_disassemble(&instruction, opcode);
    //execute
    cpu_execute(cpu, &instruction);

    //increment program counter... what if instruction is JP???
    cpu->program_counter++;
}

int cpu_execute(cpu_t *cpu, instruction_t *instruction) {
}

void cpu_free(cpu_t *cpu) {
}
