//cpu.h

#pragma once

#define DISPLAY_WIDTH 64        //x coordinate
#define DISPLAY_HEIGHT 32       //y coordinate

typedef struct cpu cpu_t;

typedef struct cpu_io_interface {
    int (*get_keyboard)();
    uint8_t (*wait_keypress)();

    bool (*get_pixel)(uint8_t x, uint8_t y);    
    void (*draw_pixel)(uint8_t x, uint8_t y, bool fill);
} cpu_io_interface_t;

cpu_t *cpu_new(const cpu_io_interface_t *cpu_io_interface);
int cpu_load(cpu_t *cpu, const rombuffer_t *rom);
int cpu_run(cpu_t *cpu);
int cpu_reset(cpu_t *cpu, const rombuffer_t *rom);
void cpu_free(cpu_t *cpu);

//for methods returning int:
//       0 - success
//      -1 - pointer is NULL
//
//    abort errors returned from cpu_exec functions:
//      -2 - value exceeds 0x0F
//      -3 - pc set to memory address < 0x200
//      -4 - attempted write to memory address < 0x200 
//      -5 - stack underflow
//      -6 - stack overflow
//      -7 - attempted execution of data
