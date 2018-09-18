//disassembler.c

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "disassembler.h"

#define MAX_ROM_SIZE 200

struct opcode {
    uint16_t bitmask;
    uint16_t filter;
    char *instruction;
};

struct romBuffer{
    uint16_t *opcodes;
    uint32_t length;
};

static int16_t var_masks[5] = {
    0x0F00,             //x
    0x00F0,             //y
    0x000F,             //n
    0x00FF,             //k or kk
    0x0FFF              //a or nnn
};

//sample opcode table
static opcode_t opcode_table[35] = {
    {
        0xF000,
        0x0000,
        "SYS a"
    },
    {
        0x00F0,
        0x00E0,
        "CLS"
    },
    {
        0x00FF,
        0x00EE,
        "RET"
    },
    {
        0xF000,
        0x1000,
        "JP a"
    },
    {
        0xF000,
        0x2000,
        "CALL a"
    },
    {
        0xF000,
        0x3000,
        "SE Vx, k"
    },
    {
        0xF000,
        0x4000,
        "SNE Vx, k"
    },
    {
        0xF000,
        0x5000,
        "SE Vx, Vy"
    },
    {
        0xF000,
        0x6000,
        "LD Vx, k"
    },
    {
        0xF000,
        0x7000,
        "ADD Vx, k"
    },
    {
        0xF00F,
        0x8000,
        "LD Vx, Vy"
    },
    {
        0xF00F,
        0x8001,
        "OR Vx, Vy"
    },
    {
        0xF00F,
        0x8002,
        "AND Vx, Vy"
    },
    {
        0xF00F,
        0x8003,
        "XOR Vx, Vy"
    },
    {
        0xF00F,
        0x8004,
        "ADD Vx, Vy"
    },
    {
        0xF00F,
        0x8005,
        "SUB Vx, Vy"
    },
    {
        0xF00F,
        0x8006,
        "SHR Vx {, Vy}"
    },
    {
        0xF00F,
        0x8007,
        "SUBN Vx, Vy"
    },
    {
        0xF00F,
        0x800E,
        "SHL Vx {, Vy}"
    },
    {
        0xF00F,
        0x9000,
        "SNE Vx, Vy"
    },
    {
        0xF000,
        0xA000,
        "LD I, a"
    },
    {
        0xF000,
        0xB000,
        "JP V0, a"
    },
    {
        0xF000,
        0xC000,
        "RND Vx, k"
    },
    {
        0xF000,
        0xD000,
        "DRW Vx, Vy, n"
    },
    {
        0xF0FF,
        0xE09E,
        "SKP Vx"
    },
    {
        0xF0FF,
        0xE0A1,
        "SKNP Vx"
    },
    {
        0xF0FF,
        0xF007,
        "LD Vx, DT"
    },
    {
        0xF0FF,
        0xF00A,
        "LD Vx, K"
    },
    {
        0xF0FF,
        0xF015,
        "LD DT, Vx"
    },
    {
        0xF0FF,
        0xF018,
        "LD ST, Vx"
    },
    {
        0xF0FF,
        0xF01E,
        "ADD I, Vx"
    },
    {
        0xF0FF,
        0xF029,
        "LD F, Vx"
    },
    {
        0xF0FF,
        0xF033,
        "LD B, Vx"
    },
    {
        0xF0FF,
        0xF055,
        "LD [I], Vx"
    },
    {
        0xF0FF,
        0xF065,
        "LD Vx, [I]"
    },
};

int main(void) {
    FILE *cnct4 = fopen("./c8games/CONNECT4", "r");

    if (cnct4 == NULL) {
        printf("error opening file\n");
        return -1;
    }

    romBuffer_t *cnct4_opcodes = CHIP8_getOpcodes(cnct4);
    disassembler_dump(cnct4_opcodes);
    romBuffer_free(cnct4_opcodes);
    
    return 0;
}

romBuffer_t *CHIP8_getOpcodes(FILE *game_f) {
    if (game_f == NULL) {
        return NULL;
    }

    uint32_t byte_count = 0;
    uint32_t byte_buffer;
    uint16_t opcode;

    romBuffer_t *rom = malloc(sizeof(romBuffer_t));
    if (rom == NULL) {
        return NULL;
    }

    rom->opcodes = malloc(MAX_ROM_SIZE * sizeof(uint16_t));
    if (rom->opcodes == NULL) {
        return NULL;
    }

    while ((byte_buffer = fgetc(game_f)) != EOF) {
        byte_count ++;
        if (byte_count % 2) {
            opcode = byte_buffer << 8;
        } else {
            opcode ^= byte_buffer;
            rom->opcodes[byte_count / 2 - 1] = opcode;
        }
    }

    rom->length = byte_count / 2;
    return rom;
}

void romBuffer_free(romBuffer_t *rom) {
    if (rom == NULL) {
        return;
    }

    free(rom->opcodes);
    free(rom);
}

void disassembler_dump(const romBuffer_t *opcodes) {
    if (opcodes == NULL) {
        return;
    }

    uint16_t opcode;
    opcode_t *id;
    for (int i = 0; i < opcodes->length; i++) {
        opcode = opcodes->opcodes[i];
        printf("opcode #%02d: %x    ", i, opcode);
        id = disassembler_filter(opcode);
        if (id == NULL) {
            printf("no matching opcode\n");
            continue;
        }
        disassembler_print(opcode, id->instruction);
    }
}

opcode_t *disassembler_filter(uint16_t opcode) {
    opcode_t *lookup;

    for (int i = 0; i < 35; i++) {
        lookup = opcode_table + i;

        if ((opcode & lookup->bitmask) == lookup->filter) {
            return lookup;
        }
    }

    return NULL;
}

bool disassembler_print(uint16_t opcode, const char *instruction) {
    char c;
    uint16_t variable;
    for (int i = 0; instruction[i] != '\0'; i++) {
        c = instruction[i];

        if (c >= 97 && c <= 172) {
            variable = disassembler_fetchVar(c, opcode);
            printf("%x", variable);
        } else {
            printf("%c", c);
        }
    }
    printf("\n");
}

uint16_t disassembler_fetchVar(char type, uint16_t opcode) {
    uint16_t variable;

    if (type == 'x') {
       variable = (opcode & var_masks[0]) >> 8;
    } else if (type == 'y') {
       variable = (opcode & var_masks[1]) >> 4;
    } else if (type == 'n') {
       variable = opcode & var_masks[2];
    } else if (type == 'k') {
       variable = opcode & var_masks[3];
    } else if (type == 'a') {
       variable = opcode & var_masks[4];
    }

    return variable;
}
