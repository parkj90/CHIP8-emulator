//cpu.h

#include <stdint.h>

typedef struct cpu cpu_t;

//for methods returning int:
//      -1: pointer is NULL
cpu_t *cpu_new();
int cpu_load(cpu_t *cpu, const rombuffer_t *rom);
int cpu_run(cpu_t *cpu);
int cpu_reset(cpu_t *cpu, const rombuffer_t *rom);
void cpu_free(cpu_t *cpu);
