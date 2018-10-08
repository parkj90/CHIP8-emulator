//cpu.h

#include <stdint.h>

typedef struct cpu cpu_t;

typedef struct cpu_io_interface {
    uint16_t (*get_keyboard)(bool blocking);

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
//      -3 - pc set to memory address < 0x1FF
//      -4 - attempted write to memory address < 0x1FF 
//      -5 - attempted execution of data
