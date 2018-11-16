//main.c

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "rombuffer.h"
#include "disassembler.h"
#include "cpu.h"
#include "sdl_io.h"

#define ROM_DIRECTORY "../c8games/"

static bool quit_signal = false;
static pthread_mutex_t mutex_quit = PTHREAD_MUTEX_INITIALIZER;

static void *cpu_thread_function(void *cpu);
static void *ui_thread_function(void *dummy_arg);

int main(int argc, char *argv[]) {
    int opt;
    bool disassembly = false;
    while ((opt = getopt(argc, argv, "d")) != -1) {
        if (opt == 'd') {
            disassembly = true;
        } else {
            fprintf(stderr, "Usage: %s [-l] rom\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Usage: %s [-l] rom\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *rom_name = argv[optind];
    char rom_path[strlen(rom_name) + strlen(ROM_DIRECTORY) + 1];
    sprintf(rom_path, "%s%s", ROM_DIRECTORY, rom_name);

    FILE *rom = fopen(rom_path, "r");
    if (rom == NULL) {
        perror("Error: ");
        exit(EXIT_FAILURE);
    }

    rombuffer_t *rom_opcodes = rombuffer_read(rom);
    if (rom_opcodes == NULL) {
        perror("Error: ");
        exit(EXIT_FAILURE);
    } 
    fclose(rom);

    // if -d option enabled, print disassembled instructions
    // and terminate emulator
    if (disassembly) {
        printf("%s disassembly: \n", rom_name);
        disassembler_dump(rom_opcodes);
        rombuffer_free(rom_opcodes);

        exit(EXIT_SUCCESS);
    }

    // begin default emulator behavior
    cpu_t *cpu = cpu_new(&sdl_io_interface);
    if (cpu == NULL) {
        fprintf(stderr, "Error: unable to initialize CPU\n");
        exit(EXIT_FAILURE);
    }

    cpu_load(cpu, rom_opcodes);

    pthread_t cpu_thread, ui_thread;
    int cpu_thread_ret, ui_thread_ret;

    sdl_io_init(rom_name);

    ui_thread_ret = pthread_create(&ui_thread, NULL, ui_thread_function, NULL);
    if (ui_thread_ret) {
        fprintf(stderr, "error: pthread_create() returns: %d\n",
                ui_thread_ret);
    }

    cpu_thread_ret = pthread_create(&cpu_thread, NULL,
                                    cpu_thread_function, (void *)cpu); 
    if (cpu_thread_ret) {
        fprintf(stderr, "error: pthread_create() returns: %d\n",
                cpu_thread_ret);
    }

    pthread_join(ui_thread, NULL);
    pthread_mutex_lock(&mutex_quit);
    quit_signal = true;
    pthread_mutex_unlock(&mutex_quit);

    int *cpu_error_ptr;
    pthread_join(cpu_thread, (void *)&cpu_error_ptr);

    sdl_io_terminate();
    cpu_free(cpu);
    rombuffer_free(rom_opcodes);

    if (*cpu_error_ptr) {
        fprintf(stderr, "CPU error: %d\n", *cpu_error_ptr);
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
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
    sdl_ui_run();

    return NULL;
}
