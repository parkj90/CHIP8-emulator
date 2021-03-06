//disassembler.c

#include <stdio.h>
#include <string.h>
#include "rombuffer.h"
#include "disassembler.h"

#define MAX_FORMATTED_OP_SIZE 6
#define FORMATTED_INSTRUCTION_SIZE 20

static const uint16_t operand_masks[][2] = {
    [OPERAND_VX]    = {0x0F00, 8},        //x
    [OPERAND_VY]    = {0x00F0, 4},        //y
    [OPERAND_N]     = {0x000F, 0},        //n
    [OPERAND_KK]    = {0x00FF, 0},        //kk
    [OPERAND_NNN]   = {0x0FFF, 0},        //nnn
    [OPERAND_DATA]  = {0xFFFF, 0}
};

static const instruction_info_t instruction_info_table[] = {
    {
        INSTRUCTION_SYS_NNN,
        0xFFFF,
        0x0000,
        "SYS",
        1,
        {OPERAND_NNN}
    },
    {
        INSTRUCTION_CLS,
        0xFFFF,
        0x00E0,
        "CLS",
        0,
        {}
    },
    {
        INSTRUCTION_RET,
        0x00FF,
        0x00EE,
        "RET",
        0,
        {}
    },
    {
        INSTRUCTION_JP_NNN,
        0xF000,
        0x1000,
        "JP",
        1,
        {OPERAND_NNN}
    },
    {
        INSTRUCTION_CALL_NNN,
        0xF000,
        0x2000,
        "CALL",
        1,
        {OPERAND_NNN}
    },
    {
        INSTRUCTION_SE_VX_KK,
        0xF000,
        0x3000,
        "SE",
        2,
        {OPERAND_VX, OPERAND_KK}
    },
    {
        INSTRUCTION_SNE_VX_KK,
        0xF000,
        0x4000,
        "SNE",
        2,
        {OPERAND_VX, OPERAND_KK}
    },
    {
        INSTRUCTION_SE_VX_VY,
        0xF000,
        0x5000,
        "SE",
        2,
        {OPERAND_VX, OPERAND_VY}
    },
    {
        INSTRUCTION_LD_VX_KK,
        0xF000,
        0x6000,
        "LD",
        2,
        {OPERAND_VX, OPERAND_KK}
    },
    {
        INSTRUCTION_ADD_VX_KK,
        0xF000,
        0x7000,
        "ADD",
        2,
        {OPERAND_VX, OPERAND_KK}
    },
    {
        INSTRUCTION_LD_VX_VY,
        0xF00F,
        0x8000,
        "LD",
        2,
        {OPERAND_VX, OPERAND_VY}
    },
    {
        INSTRUCTION_OR_VX_VY,
        0xF00F,
        0x8001,
        "OR",
        2,
        {OPERAND_VX, OPERAND_VY}
    },
    {
        INSTRUCTION_AND_VX_VY,
        0xF00F,
        0x8002,
        "AND",
        2,
        {OPERAND_VX, OPERAND_VY}
    },
    {
        INSTRUCTION_XOR_VX_VY,
        0xF00F,
        0x8003,
        "XOR",
        2,
        {OPERAND_VX, OPERAND_VY}
    },
    {
        INSTRUCTION_ADD_VX_VY,
        0xF00F,
        0x8004,
        "ADD",
        2,
        {OPERAND_VX, OPERAND_VY}
    },
    {
        INSTRUCTION_SUB_VX_VY,
        0xF00F,
        0x8005,
        "SUB",
        2,
        {OPERAND_VX, OPERAND_VY}
    },
    {
        INSTRUCTION_SHR_VX_VY,
        0xF00F,
        0x8006,
        "SHR",
        2,
        {OPERAND_VX, OPERAND_VY}
    },
    {
        INSTRUCTION_SUBN_VX_VY,
        0xF00F,
        0x8007,
        "SUBN",
        2,
        {OPERAND_VX, OPERAND_VY}
    },
    {
        INSTRUCTION_SHL_VX_VY,
        0xF00F,
        0x800E,
        "SHL",
        2,
        {OPERAND_VX, OPERAND_VY}
    },
    {
        INSTRUCTION_SNE_VX_VY,
        0xF00F,
        0x9000,
        "SNE",
        2,
        {OPERAND_VX, OPERAND_VY}
    },
    {
        INSTRUCTION_LD_I_NNN,
        0xF000,
        0xA000,
        "LD",
        2,
        {OPERAND_I, OPERAND_NNN}
    },
    {
        INSTRUCTION_JP_V0_NNN,
        0xF000,
        0xB000,
        "JP",
        2,
        {OPERAND_V0, OPERAND_NNN}
    },
    {
        INSTRUCTION_RND_VX_KK,
        0xF000,
        0xC000,
        "RND",
        2,
        {OPERAND_VX, OPERAND_KK}
    },
    {
        INSTRUCTION_DRW_VX_VY_N,
        0xF000,
        0xD000,
        "DRW",
        3,
        {OPERAND_VX, OPERAND_VY, OPERAND_N}
    },
    {
        INSTRUCTION_SKP_VX,
        0xF0FF,
        0xE09E,
        "SKP",
        1,
        {OPERAND_VX}
    },
    {
        INSTRUCTION_SKNP_VX,
        0xF0FF,
        0xE0A1,
        "SKNP",
        1,
        {OPERAND_VX}
    },
    {
        INSTRUCTION_LD_VX_DT,
        0xF0FF,
        0xF007,
        "LD",
        2,
        {OPERAND_VX, OPERAND_DT}
    },
    {
        INSTRUCTION_LD_VX_K,
        0xF0FF,
        0xF00A,
        "LD",
        2,
        {OPERAND_VX, OPERAND_K}
    },
    {
        INSTRUCTION_LD_DT_VX,
        0xF0FF,
        0xF015,
        "LD",
        2,
        {OPERAND_DT, OPERAND_VX}
    },
    {
        INSTRUCTION_LD_ST_VX,
        0xF0FF,
        0xF018,
        "LD",
        2,
        {OPERAND_ST, OPERAND_VX}
    },
    {
        INSTRUCTION_ADD_I_VX,
        0xF0FF,
        0xF01E,
        "ADD",
        2,
        {OPERAND_I, OPERAND_VX}
    },
    {
        INSTRUCTION_LD_F_VX,
        0xF0FF,
        0xF029,
        "LD",
        2,
        {OPERAND_F, OPERAND_VX}
    },
    {
        INSTRUCTION_LD_B_VX,
        0xF0FF,
        0xF033,
        "LD",
        2,
        {OPERAND_B, OPERAND_VX}
    },
    {
        INSTRUCTION_LD_I_VX,
        0xF0FF,
        0xF055,
        "LD",
        2,
        {OPERAND_I, OPERAND_VX}
    },
    {
        INSTRUCTION_LD_VX_I,
        0xF0FF,
        0xF065,
        "LD",
        2,
        {OPERAND_VX, OPERAND_I}
    },
    {
        INSTRUCTION_DATA,
        0x0000,
        0x0000,
        ".data",
        1,
        {OPERAND_DATA}
    }
};

static const size_t instruction_info_table_size =
    sizeof(instruction_info_table) / sizeof(instruction_info_table[0]);

int disassembler_dump(const rombuffer_t *rom) {
    if (rom == NULL) {
        return -1;
    }

    for (unsigned int i = 0; i < rom->length; i++) {
        uint16_t opcode = rom->data[i];

        instruction_t instruction;
        disassembler_disassemble(&instruction, opcode);

        char formatted_instruction[FORMATTED_INSTRUCTION_SIZE];
        int format_error = disassembler_format(formatted_instruction,
                                               sizeof(formatted_instruction),
                                               &instruction);
        if (format_error) {
            return format_error;
        }

        printf("0x%03x: %04x    %s\n", i, opcode, formatted_instruction);
    }

    return 0;
}

int disassembler_disassemble(instruction_t *instruction, const uint16_t opcode) {
    if (instruction == NULL) {
        return -1;
    }

    const instruction_info_t *instruction_info = disassembler_lookup(opcode);
    instruction->instruction_info = instruction_info;

    for (size_t i = 0; i < instruction_info->operand_count; i++) {
        uint16_t mask = operand_masks[instruction_info->operand_types[i]][0];
        uint16_t shift = operand_masks[instruction_info->operand_types[i]][1];
        instruction->operands[i] = (opcode & mask) >> shift;
    }

    return 0;
}

int disassembler_format(char *formatted_instruction, size_t size,
                        const instruction_t *instruction) {
    const instruction_info_t *instruction_info = instruction->instruction_info;
    size_t form_inst_size = snprintf(formatted_instruction, size,
                                     "%s", instruction_info->mnemonic);
    
    const uint16_t *operands = instruction->operands;
    for (size_t i = 0; i < instruction_info->operand_count; i++) {
        if (size < form_inst_size) {
            return -2;
        }

        char formatted_operand[MAX_FORMATTED_OP_SIZE];
        size_t form_op_size;
        switch (instruction_info->operand_types[i]) {
            case OPERAND_VX:
                form_op_size = snprintf(formatted_operand,
                                        MAX_FORMATTED_OP_SIZE,
                                        " V%x", operands[i]);
                break;
            case OPERAND_VY:
                form_op_size = snprintf(formatted_operand,
                                        MAX_FORMATTED_OP_SIZE,
                                        " V%x", operands[i]);
                break;
            case OPERAND_N:
                form_op_size = snprintf(formatted_operand,
                                        MAX_FORMATTED_OP_SIZE,
                                        " %x", operands[i]);
                break;
            case OPERAND_KK:
                form_op_size = snprintf(formatted_operand,
                                        MAX_FORMATTED_OP_SIZE,
                                        " %x", operands[i]);
                break;
            case OPERAND_NNN:
                form_op_size = snprintf(formatted_operand,
                                        MAX_FORMATTED_OP_SIZE,
                                        " %x", operands[i]);
                break;
            case OPERAND_I:
                form_op_size = snprintf(formatted_operand,
                                        MAX_FORMATTED_OP_SIZE,
                                        " I");
                break;
            case OPERAND_V0:
                form_op_size = snprintf(formatted_operand,
                                        MAX_FORMATTED_OP_SIZE,
                                        " V0");
                break;
            case OPERAND_DT:
                form_op_size = snprintf(formatted_operand,
                                        MAX_FORMATTED_OP_SIZE,
                                        " DT");
                break;
            case OPERAND_ST:
                form_op_size = snprintf(formatted_operand,
                                        MAX_FORMATTED_OP_SIZE,
                                        " ST");
                break;
            case OPERAND_F:
                form_op_size = snprintf(formatted_operand,
                                        MAX_FORMATTED_OP_SIZE,
                                        " F");
                break;
            case OPERAND_B:
                form_op_size = snprintf(formatted_operand,
                                        MAX_FORMATTED_OP_SIZE,
                                        " B");
                break;
            case OPERAND_K:
                form_op_size = snprintf(formatted_operand,
                                        MAX_FORMATTED_OP_SIZE,
                                        " K");
                break;
            case OPERAND_DATA:
                form_op_size = snprintf(formatted_operand,
                                        MAX_FORMATTED_OP_SIZE,
                                        " %x", operands[i]);
                break;
            default:
                return -3;
        }

        if (i < instruction_info->operand_count - 1) {
            form_op_size = snprintf(formatted_operand + form_op_size,
                                    MAX_FORMATTED_OP_SIZE - form_op_size, ",");
        }

        form_inst_size += snprintf(formatted_instruction + form_inst_size,
                                   size - form_inst_size,
                                   "%s",
                                   formatted_operand);
    }

    return 0;
}

const instruction_info_t *disassembler_lookup(uint16_t opcode) {
    for (size_t i = 0; i < instruction_info_table_size; i++) {
        const instruction_info_t *instruction_info = &instruction_info_table[i];

        if ((opcode & instruction_info->mask) == instruction_info->id) {
            return instruction_info;
        }
    }
    
    return NULL;
}
