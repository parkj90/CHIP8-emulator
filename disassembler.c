//disassembler.c

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "rombuffer.h"
#include "disassembler.h"

#define MAX_FORMATTED_OP_SIZE 6

typedef struct instruction {
    uint16_t mask;
    uint16_t id;
    char *mnemonic;

    size_t operand_count;
    uint16_t operand_types[3];
} instruction_t;

enum operand_id {
    OPERAND_VX  = 0,
    OPERAND_VY  = 1,
    OPERAND_N   = 2,
    OPERAND_KK  = 3,
    OPERAND_NNN = 4,
    OPERAND_I   = 5,
    OPERAND_V0  = 6,
    OPERAND_DT  = 7,
    OPERAND_ST  = 8,
    OPERAND_K   = 9
};

static uint16_t operand_masks[][2] = {
    {0x0F00, 8},        //x
    {0x00F0, 4},        //y
    {0x000F, 0},        //n
    {0x00FF, 0},        //kk
    {0x0FFF, 0}         //nnn
};

//sample opcode table
static const instruction_t instruction_table[] = {
    {
        0xF000,
        0x0000,
        "SYS",
        1,
        {OPERAND_NNN},
    },
    {
        0x00F0,
        0x00E0,
        "CLS",
        0,
        {}
    },
    {
        0x00FF,
        0x00EE,
        "RET",
        0,
        {}
    },
    {
        0xF000,
        0x1000,
        "JP",
        1,
        {OPERAND_NNN}
    },
    {
        0xF000,
        0x2000,
        "CALL",
        1,
        {OPERAND_NNN}
    },
    {
        0xF000,
        0x3000,
        "SE",
        2,
        {OPERAND_VX, OPERAND_KK}
    },
    {
        0xF000,
        0x4000,
        "SNE",
        2,
        {OPERAND_VX, OPERAND_KK}
    },
    {
        0xF000,
        0x5000,
        "SE",
        2,
        {OPERAND_VX, OPERAND_VY}
    },
    {
        0xF000,
        0x6000,
        "LD",
        2,
        {OPERAND_VX, OPERAND_KK}
    },
    {
        0xF000,
        0x7000,
        "ADD",
        2,
        {OPERAND_VX, OPERAND_KK}
    },
    {
        0xF00F,
        0x8000,
        "LD",
        2,
        {OPERAND_VX, OPERAND_VY}
    },
    {
        0xF00F,
        0x8001,
        "OR",
        2,
        {OPERAND_VX, OPERAND_VY}
    },
    {
        0xF00F,
        0x8002,
        "AND",
        2,
        {OPERAND_VX, OPERAND_VY}
    },
    {
        0xF00F,
        0x8003,
        "XOR",
        2,
        {OPERAND_VX, OPERAND_VY}
    },
    {
        0xF00F,
        0x8004,
        "ADD",
        2,
        {OPERAND_VX, OPERAND_VY}
    },
    {
        0xF00F,
        0x8005,
        "SUB",
        2,
        {OPERAND_VX, OPERAND_VY}
    },
    {
        0xF00F,
        0x8006,
        "SHR",
        2,
        {OPERAND_VX, OPERAND_VY}
    },
    {
        0xF00F,
        0x8007,
        "SUBN",
        2,
        {OPERAND_VX, OPERAND_VY}
    },
    {
        0xF00F,
        0x800E,
        "SHL",
        2,
        {OPERAND_VX, OPERAND_VY}
    },
    {
        0xF00F,
        0x9000,
        "SNE",
        2,
        {OPERAND_VX, OPERAND_VY}
    },
    {
        0xF000,
        0xA000,
        "LD",
        2,
        {OPERAND_I, OPERAND_NNN}
    },
    {
        0xF000,
        0xB000,
        "JP",
        2,
        {OPERAND_V0, OPERAND_NNN}
    },
    {
        0xF000,
        0xC000,
        "RND",
        2,
        {OPERAND_VX, OPERAND_KK}
    },
    {
        0xF000,
        0xD000,
        "DRW",
        3,
        {OPERAND_VX, OPERAND_VY, OPERAND_N}
    },
    {
        0xF0FF,
        0xE09E,
        "SKP",
        1,
        {OPERAND_VX}
    },
    {
        0xF0FF,
        0xE0A1,
        "SKNP",
        1,
        {OPERAND_VX}
    },
    {
        0xF0FF,
        0xF007,
        "LD",
        2,
        {OPERAND_VX, OPERAND_DT}
    },
    {
        0xF0FF,
        0xF00A,
        "LD",
        2,
        {OPERAND_VX, OPERAND_KK}
    },
    {
        0xF0FF,
        0xF015,
        "LD",
        2,
        {OPERAND_DT, OPERAND_VX}
    },
    {
        0xF0FF,
        0xF018,
        "LD",
        2,
        {OPERAND_ST, OPERAND_VX}
    },
    {
        0xF0FF,
        0xF01E,
        "ADD",
        2,
        {OPERAND_I, OPERAND_VX}
    },
    {
        0xF0FF,
        0xF029,
        "LD",
        2,
        {OPERAND_I, OPERAND_VX}
    },
    {
        0xF0FF,
        0xF033,
        "LD",
        2,
        {OPERAND_I, OPERAND_VX}
    },
    {
        0xF0FF,
        0xF055,
        "LD",
        2,
        {OPERAND_I, OPERAND_VX}
    },
    {
        0xF0FF,
        0xF065,
        "LD",
        2,
        {OPERAND_VX, OPERAND_I}
    },
    {
        0x0000,
        0x0000,
        "no matching instruction",
        0,
        {}
    }
};

static const int instruction_table_size = sizeof(instruction_table)/sizeof(instruction_table[0]);

void disassembler_dump(const rombuffer_t *rom) {
    if (rom == NULL) {
        return;
    }

    for (size_t i = 0; i < rom->length; i++) {
        uint16_t opcode = rom->data[i];
        printf("0x%03x: %04x    ", (unsigned int)i, opcode);

        const instruction_t *instruction = disassembler_lookup(opcode);

        uint16_t operands[instruction->operand_count];
        disassembler_fetch_operands(operands, opcode, instruction);

        size_t formatted_operands_size = 12;
        char formatted_operands[formatted_operands_size];
        disassembler_format_operands(formatted_operands, formatted_operands_size, operands, instruction);

        size_t formatted_instruction_size = 20;
        char formatted_instruction[formatted_instruction_size];
        disassembler_format(formatted_instruction, formatted_instruction_size, instruction->mnemonic, formatted_operands);

        printf("%s\n", formatted_instruction);
    }
}

const instruction_t *disassembler_lookup(uint16_t opcode) {
    for (int i = 0; i < instruction_table_size; i++) {
        const instruction_t *instruction = &instruction_table[i];

        if ((opcode & instruction->mask) == instruction->id) {
            return instruction;
        }
    }
    
    return NULL;
}

void disassembler_fetch_operands(uint16_t *operands, uint16_t opcode, const instruction_t *instruction) {
    if (operands == NULL) {
        return;
    }

    for (size_t i = 0; i < instruction->operand_count; i++) {
        uint16_t mask = operand_masks[instruction->operand_types[i]][0];
        uint16_t shift = operand_masks[instruction->operand_types[i]][1];
        operands[i] = opcode & mask >> shift;
    }
}

void disassembler_format_operands(char *formatted_operands, size_t size, const uint16_t *operands, const instruction_t *instruction) {
    for (size_t i = 0; i < instruction->operand_count; i++) {
        char formatted_operand[MAX_FORMATTED_OP_SIZE];
        switch (instruction->operand_types[i]) {
            case OPERAND_VX:
                snprintf(formatted_operand, MAX_FORMATTED_OP_SIZE, " V%x", operands[i]);
                break;
            case OPERAND_VY:
                snprintf(formatted_operand, MAX_FORMATTED_OP_SIZE, " V%x", operands[i]);
                break;
            case OPERAND_N:
                snprintf(formatted_operand, MAX_FORMATTED_OP_SIZE, " %x", operands[i]);
                break;
            case OPERAND_KK:
                snprintf(formatted_operand, MAX_FORMATTED_OP_SIZE, " %x", operands[i]);
                break;
            case OPERAND_NNN:
                snprintf(formatted_operand, MAX_FORMATTED_OP_SIZE, " %x", operands[i]);
                break;
            case OPERAND_I:
                snprintf(formatted_operand, MAX_FORMATTED_OP_SIZE, " I");
                break;
            case OPERAND_V0:
                snprintf(formatted_operand, MAX_FORMATTED_OP_SIZE, " V0");
                break;
            case OPERAND_DT:
                snprintf(formatted_operand, MAX_FORMATTED_OP_SIZE, " DT");
                break;
            case OPERAND_ST:
                snprintf(formatted_operand, MAX_FORMATTED_OP_SIZE, " ST");
                break;
            case OPERAND_K:
                snprintf(formatted_operand, MAX_FORMATTED_OP_SIZE, " K");
                break;
            default:
                perror("no matching operand enum");
                return;
        }
        if (i < instruction->operand_count - 1) {
            size_t formatted_op_size = strlen(formatted_operand);
            snprintf(formatted_operand + formatted_op_size, MAX_FORMATTED_OP_SIZE - formatted_op_size, ",");
        }
        size_t formatted_ops_size = strlen(formatted_operands);
        snprintf(formatted_operands + formatted_ops_size, size - formatted_ops_size, "%s", formatted_operand);
    }
}

void disassembler_format(char *formatted_instruction, size_t size, const char *mnemonic, const char *formatted_operands) {
    snprintf(formatted_instruction, size, "%s%s", mnemonic, formatted_operands);
}
