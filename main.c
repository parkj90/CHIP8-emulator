//main.c

#include <stdio.h>
#include "rombuffer.h"
#include "disassembler.h"
#include "cpu.h"

int main(void) {
    FILE *cnct4 = fopen("../c8games/CONNECT4", "r");
    if (cnct4 == NULL) {
        perror("Error: ");
        return -1;
    }

    rombuffer_t *cnct4_opcodes = rombuffer_read(cnct4);
    if (cnct4_opcodes == NULL) {
        perror("Error: ");
        return -1;
    } 

    cpu_t *cpu = cpu_new();
    if (cpu == NULL) {
        return -1;
    }
    cpu_load(cpu, cnct4_opcodes);

    cpu_run(cpu);

    cpu_free(cpu);
    rombuffer_free(cnct4_opcodes);
    
    return 0;
}
