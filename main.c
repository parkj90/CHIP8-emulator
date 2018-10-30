//main.c

#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include "rombuffer.h"
#include "disassembler.h"
#include "cpu.h"
#include "ncurses_io.h"

static int error_code = 0;

static void *cpu_thread_function(void *cpu);

int main(void) {
    FILE *rom = fopen("../c8games/CONNECT4", "r");
    const char *rom_name = "CONNECT4";

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

    cpu_load(cpu, rom_opcodes);

    pthread_t cpu_thread;
    int cpu_thread_ret;

    ncurses_io_init();

    cpu_thread_ret = pthread_create(&cpu_thread, NULL, cpu_thread_function, (void *)cpu); 
    if (cpu_thread_ret) {
        fprintf(stderr, "error: pthread_create() returns: %d\n", cpu_thread_ret);
    }

    pthread_join(cpu_thread, NULL);

    ncurses_io_terminate();
    cpu_free(cpu);
    rombuffer_free(rom_opcodes);

    if (error_code) {
        printf("%d\n", error_code);
    }

    return error_code;
}

static void *cpu_thread_function(void *cpu) {
    int us_counter = 0;
    while (!error_code) {
        usleep(100);
        us_counter++;
        if (us_counter >= 166) {
            us_counter = 0;
            cpu_decrement_timers(cpu);
        }

        error_code = cpu_execute((cpu_t *) cpu);
    }

    return NULL;
}
