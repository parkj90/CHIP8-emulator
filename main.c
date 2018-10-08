//main.c

#include <stdio.h>
#include <stdbool.h>
#include "rombuffer.h"
#include "disassembler.h"
#include "cpu.h"

//fix me: temporary filler functions, running main will segfault
static uint16_t (*get_keyboard)(bool blocking);
static bool (*get_pixel)(uint8_t x, uint8_t y);
static void (*draw_pixel)(uint8_t x, uint8_t y, bool fill);

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

    cpu_io_interface_t cpu_io_interface = {
        get_keyboard, get_pixel, draw_pixel
    };

    cpu_t *cpu = cpu_new(&cpu_io_interface);
    if (cpu == NULL) {
        return -1;
    }

    //fixme add keyboard and display functions
    cpu_load(cpu, cnct4_opcodes);

    cpu_run(cpu);

    cpu_free(cpu);
    rombuffer_free(cnct4_opcodes);
    
    return 0;
}
