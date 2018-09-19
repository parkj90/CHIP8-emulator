//disassembler.c

#include <stdio.h>
#include <stdint.h>
#include "rombuffer.h"
#include "disassembler.h"

struct instruction {
    uint16_t mask;
    uint16_t id;
    char *mnemonic;

    size_t operand_count;
    uint16_t operand_enums[3];
};

static uint16_t operand_masks[5][2] = {
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
        "SYS ",
        1,
        {4},
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
        "JP ",
        1,
        {4}
    },
    {
        0xF000,
        0x2000,
        "CALL ",
        1,
        {4}
    },
    {
        0xF000,
        0x3000,
        "SE ",
        2,
        {0, 3}
    },
    {
        0xF000,
        0x4000,
        "SNE ",
        2,
        {0, 3}
    },
    {
        0xF000,
        0x5000,
        "SE ",
        2,
        {0, 1}
    },
    {
        0xF000,
        0x6000,
        "LD ",
        2,
        {0, 3}
    },
    {
        0xF000,
        0x7000,
        "ADD ",
        2,
        {0, 3}
    },
    {
        0xF00F,
        0x8000,
        "LD ",
        2,
        {0, 1}
    },
    {
        0xF00F,
        0x8001,
        "OR ",
        2,
        {0, 1}
    },
    {
        0xF00F,
        0x8002,
        "AND ",
        2,
        {0, 1}
    },
    {
        0xF00F,
        0x8003,
        "XOR ",
        2,
        {0, 1}
    },
    {
        0xF00F,
        0x8004,
        "ADD ",
        2,
        {0, 1}
    },
    {
        0xF00F,
        0x8005,
        "SUB ",
        2,
        {0, 1}
    },
    {
        0xF00F,
        0x8006,
        "SHR ",
        2,
        {0, 1}
    },
    {
        0xF00F,
        0x8007,
        "SUBN ",
        2,
        {0, 1}
    },
    {
        0xF00F,
        0x800E,
        "SHL ",
        2,
        {0, 1}
    },
    {
        0xF00F,
        0x9000,
        "SNE ",
        2,
        {0, 1}
    },
    {
        0xF000,
        0xA000,
        "LD ",
        2,
        {5, 4}
    },
    {
        0xF000,
        0xB000,
        "JP ",
        2,
        {6, 4}
    },
    {
        0xF000,
        0xC000,
        "RND ",
        2,
        {0, 3}
    },
    {
        0xF000,
        0xD000,
        "DRW ",
        3,
        {0, 1, 2}
    },
    {
        0xF0FF,
        0xE09E,
        "SKP ",
        1,
        {0}
    },
    {
        0xF0FF,
        0xE0A1,
        "SKNP ",
        1,
        {0}
    },
    {
        0xF0FF,
        0xF007,
        "LD ",
        2,
        {0, 7}
    },
    {
        0xF0FF,
        0xF00A,
        "LD ",
        2,
        {0, 9}
    },
    {
        0xF0FF,
        0xF015,
        "LD ",
        2,
        {7, 0}
    },
    {
        0xF0FF,
        0xF018,
        "LD ",
        2,
        {8, 0}
    },
    {
        0xF0FF,
        0xF01E,
        "ADD ",
        2,
        {5, 0}
    },
    {
        0xF0FF,
        0xF029,
        "LD ",
        2,
        {5, 0}
    },
    {
        0xF0FF,
        0xF033,
        "LD ",
        2,
        {5, 0}
    },
    {
        0xF0FF,
        0xF055,
        "LD ",
        2,
        {5, 0}
    },
    {
        0xF0FF,
        0xF065,
        "LD ",
        2,
        {0, 5}
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

void disassembler_dump(const rombuffer_t *opcodes) {
    if (opcodes == NULL) {
        return;
    }

    uint16_t opcode;
    const instruction_t *instruction;
    for (int i = 0; i < opcodes->length; i++) {
        opcode = opcodes->opcodes[i];
        printf("opcode #%02d: %x    ", i, opcode);
        instruction = disassembler_lookup(opcode);
        disassembler_print(opcode, instruction);
    }
}

const instruction_t *disassembler_lookup(uint16_t opcode) {
    const instruction_t *instruction;

    for (int i = 0; i < instruction_table_size; i++) {
        instruction = &instruction_table[i];

        if ((opcode & instruction->mask) == instruction->id) {
            return instruction;
        }
    }
}

bool disassembler_print(uint16_t opcode, const instruction_t *instruction) {
    uint16_t enumeration;
    uint16_t operand;

    printf("%s", instruction->mnemonic);
    for (size_t i = 0; i < instruction->operand_count; i++) {
        enumeration = instruction->operand_enums[i];

        switch(enumeration) {
            case 5:
                printf("I");
                break;
            case 6:
                printf("V0");
                break;
            case 7:
                printf("DT");
                break;
            case 8:
                printf("ST");
                break;
            case 9:
                printf("K");
                break;
            default:
                operand = disassembler_fetch_operand(opcode, enumeration);

                if (enumeration < 2) {
                    printf("V");
                }
                printf("%x", operand);
        }

        if (i < instruction->operand_count - 1) {
            printf(", ");
        }
    }
    printf("\n");
}

uint16_t disassembler_fetch_operand(uint16_t opcode, uint16_t enumeration) {
    uint16_t operand = opcode & operand_masks[enumeration][0] >> operand_masks[enumeration][1];

    return operand;
}
