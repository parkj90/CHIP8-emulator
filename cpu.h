//cpu.h

#include <stdint.h>

typedef struct cpu cpu_t;

//for methods returning int:
//      -1: pointer is NULL
cpu_t *cpu_new();
int cpu_load(cpu_t *cpu, rombuffer_t *rom);
int cpu_run(cpu_t *cpu);
int cpu_execute(cpu_t *cpu, instruction_t *instruction);
//void cpu_reset(cpu_t *cpu);
void cpu_free(cpu_t *cpu);
