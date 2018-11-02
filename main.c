//main.c

#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include "rombuffer.h"
#include "disassembler.h"
#include "cpu.h"
#include "ncurses_io.h"

static bool quit_signal = false;
pthread_mutex_t mutex_quit = PTHREAD_MUTEX_INITIALIZER;

static void *cpu_thread_function(void *cpu);
static void *ui_thread_function(void *dummy_arg);

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

    pthread_t cpu_thread, ui_thread;
    int cpu_thread_ret, ui_thread_ret;

    ncurses_io_init();

    ui_thread_ret = pthread_create(&ui_thread, NULL, ui_thread_function, NULL);
    if (ui_thread_ret) {
        fprintf(stderr, "error: pthread_create() returns: %d\n", ui_thread_ret);
    }

    cpu_thread_ret = pthread_create(&cpu_thread, NULL, cpu_thread_function, (void *)cpu); 
    if (cpu_thread_ret) {
        fprintf(stderr, "error: pthread_create() returns: %d\n", cpu_thread_ret);
    }

    int *cpu_error_ptr;

    pthread_join(ui_thread, NULL);

    pthread_mutex_lock(&mutex_quit);
    quit_signal = true;
    pthread_mutex_unlock(&mutex_quit);

    pthread_join(cpu_thread, (void *)&cpu_error_ptr);

    ncurses_io_terminate();
    cpu_free(cpu);
    rombuffer_free(rom_opcodes);

    if (*cpu_error_ptr) {
        printf("%d\n", *cpu_error_ptr);
    }

    return *cpu_error_ptr;
}

static void *cpu_thread_function(void *cpu) {
    int us_counter = 0;
    static int cpu_error = 0;

    while (!cpu_error) {
        pthread_mutex_lock(&mutex_quit);
        if (quit_signal) {
            pthread_mutex_unlock(&mutex_quit);
            break;
        }
        pthread_mutex_unlock(&mutex_quit);

        usleep(3000);

        us_counter += 1;
        if (us_counter >= 6) {
            us_counter = 0;
            cpu_decrement_timers(cpu);
        }

        cpu_error = cpu_execute((cpu_t *) cpu);
    }

    return &cpu_error;
}

static void *ui_thread_function(void *dummy_arg) {
    ncurses_ui_run();

    return NULL;
}
