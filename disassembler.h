//diassembler.h

#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum instruction_type {
    INSTRUCTION_SYS_NNN     = 0,
    INSTRUCTION_CLS         = 1,
    INSTRUCTION_RET         = 2,
    INSTRUCTION_JP_NNN      = 3,
    INSTRUCTION_CALL_NNN    = 4,
    INSTRUCTION_SE_VX_KK    = 5,
    INSTRUCTION_SNE_VX_KK   = 6,
    INSTRUCTION_SE_VX_VY    = 7,
    INSTRUCTION_LD_VX_KK    = 8,
    INSTRUCTION_ADD_VX_KK   = 9,
    INSTRUCTION_LD_VX_VY    = 10,
    INSTRUCTION_OR_VX_VY    = 11,
    INSTRUCTION_AND_VX_VY   = 12,
    INSTRUCTION_XOR_VX_VY   = 13,
    INSTRUCTION_ADD_VX_VY   = 14,
    INSTRUCTION_SUB_VX_VY   = 15,
    INSTRUCTION_SHR_VX_VY   = 16,
    INSTRUCTION_SUBN_VX_VY  = 17,
    INSTRUCTION_SHL_VX_VY   = 18,
    INSTRUCTION_SNE_VX_VY   = 19,
    INSTRUCTION_LD_I_NNN    = 20,
    INSTRUCTION_JP_V0_NNN   = 21,
    INSTRUCTION_RND_VX_KK   = 22,
    INSTRUCTION_DRW_VX_VY_N = 23,
    INSTRUCTION_SKP_VX      = 24,
    INSTRUCTION_SKNP_VX     = 25,
    INSTRUCTION_LD_VX_DT    = 26,
    INSTRUCTION_LD_VX_K     = 27,
    INSTRUCTION_LD_DT_VX    = 28,
    INSTRUCTION_LD_ST_VX    = 29,
    INSTRUCTION_ADD_I_VX    = 30,
    INSTRUCTION_LD_F_VX     = 31,
    INSTRUCTION_LD_B_VX     = 32,
    INSTRUCTION_LD_I_VX     = 33,
    INSTRUCTION_LD_VX_I     = 34,
    INSTRUCTION_DATA        = 35
} instruction_type_t;

typedef enum operand_type {
    OPERAND_VX    = 0,
    OPERAND_VY    = 1,
    OPERAND_N     = 2,
    OPERAND_KK    = 3,
    OPERAND_NNN   = 4,
    OPERAND_I     = 5,
    OPERAND_V0    = 6,
    OPERAND_DT    = 7,
    OPERAND_ST    = 8,
    OPERAND_F     = 9,
    OPERAND_B     = 10,
    OPERAND_K     = 11,
    OPERAND_DATA  = 12
} operand_type_t;

typedef struct instruction_info {
    instruction_type_t instruction_type;

    uint16_t mask;
    uint16_t id;
    char *mnemonic;

    size_t operand_count;
    operand_type_t operand_types[3];
} instruction_info_t;

typedef struct instruction {
    const instruction_info_t *instruction_info;
    uint16_t operands[3];
} instruction_t;

//for methods returning int:
//      -1: pointer is NULL
//      -2: insufficient memory
//      -3: no matching operand_type
int disassembler_dump(const rombuffer_t *opcodes);
int disassembler_disassemble(instruction_t *instruction, uint16_t opcode);
int disassembler_format(char *formatted_instruction, size_t size, const instruction_t *instruction);

const instruction_info_t *disassembler_lookup(uint16_t opcode);
