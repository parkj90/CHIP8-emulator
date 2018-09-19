//rombuffer.h

#pragma once
#include <stdint.h>

typedef struct rombuffer{
    uint16_t *opcodes;
    size_t length;
} rombuffer_t;

rombuffer_t *rombuffer_getOpcodes(FILE *game_f);
void rombuffer_free(rombuffer_t *rom);
