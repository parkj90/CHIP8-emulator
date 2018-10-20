//main.c

#include <stdio.h>
#include <stdbool.h>
#include "rombuffer.h"
#include "disassembler.h"
#include "cpu.h"
#include "ncurses_io.h"

int main(void) {
    FILE *cnct4 = fopen("../c8games/INVADERS", "r");
    if (cnct4 == NULL) {
        perror("Error: ");
        return -1;
    }

    rombuffer_t *cnct4_opcodes = rombuffer_read(cnct4);
    if (cnct4_opcodes == NULL) {
        perror("Error: ");
        return -1;
    } 

    cpu_t *cpu = cpu_new(&ncurses_io_interface);
    if (cpu == NULL) {
        return -1;
    }

    ncurses_io_init();
    cpu_load(cpu, cnct4_opcodes);

    int e;
    e = cpu_run(cpu);

    ncurses_io_terminate();
    cpu_free(cpu);
    rombuffer_free(cnct4_opcodes);

    printf("cpu_error_code: %d\n", e);


    printf("connect 4 terminated\n");
    
    return e;
}
