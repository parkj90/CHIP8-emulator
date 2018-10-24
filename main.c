//main.c

#include <stdio.h>
#include <stdbool.h>
#include "rombuffer.h"
#include "disassembler.h"
#include "cpu.h"
#include "ncurses_io.h"

int main(void) {
    FILE *rom = fopen("../c8games/INVADERS", "r");
    const char *rom_name = "INVADERS";

    if (rom == NULL) {
        perror("Error: ");
        return -1;
    }

    rombuffer_t *rom_opcodes = rombuffer_read(rom);
    if (rom_opcodes == NULL) {
        perror("Error: ");
        return -1;
    } 

    cpu_t *cpu = cpu_new(&ncurses_io_interface);
    if (cpu == NULL) {
        return -1;
    }

    printf("Starting %s...\n", rom_name);
    ncurses_io_init();
    cpu_load(cpu, rom_opcodes);

    int error_code;
    error_code = cpu_run(cpu);

    ncurses_io_terminate();
    cpu_free(cpu);
    rombuffer_free(rom_opcodes);

    if (error_code) {
        printf("%d\n", error_code);
    }
    return error_code;
}
