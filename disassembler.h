//diassembler.h

#pragma once

typedef enum instruction_type {
    INSTRUCTION_SYS_NNN,
    INSTRUCTION_CLS,
    INSTRUCTION_RET,
    INSTRUCTION_JP_NNN,
    INSTRUCTION_CALL_NNN,
    INSTRUCTION_SE_VX_KK,
    INSTRUCTION_SNE_VX_KK,
    INSTRUCTION_SE_VX_VY,
    INSTRUCTION_LD_VX_KK,
    INSTRUCTION_ADD_VX_KK,
    INSTRUCTION_LD_VX_VY,
    INSTRUCTION_OR_VX_VY,
    INSTRUCTION_AND_VX_VY,
    INSTRUCTION_XOR_VX_VY,
    INSTRUCTION_ADD_VX_VY,
    INSTRUCTION_SUB_VX_VY,
    INSTRUCTION_SHR_VX_VY,
    INSTRUCTION_SUBN_VX_VY,
    INSTRUCTION_SHL_VX_VY,
    INSTRUCTION_SNE_VX_VY,
    INSTRUCTION_LD_I_NNN,
    INSTRUCTION_JP_V0_NNN,
    INSTRUCTION_RND_VX_KK,
    INSTRUCTION_DRW_VX_VY_N,
    INSTRUCTION_SKP_VX,
    INSTRUCTION_SKNP_VX,
    INSTRUCTION_LD_VX_DT,
    INSTRUCTION_LD_VX_K,
    INSTRUCTION_LD_DT_VX,
    INSTRUCTION_LD_ST_VX,
    INSTRUCTION_ADD_I_VX,
    INSTRUCTION_LD_F_VX,
    INSTRUCTION_LD_B_VX,
    INSTRUCTION_LD_I_VX,
    INSTRUCTION_LD_VX_I,
    INSTRUCTION_DATA        
} instruction_type_t;

typedef enum operand_type {
    OPERAND_VX,
    OPERAND_VY,
    OPERAND_N,
    OPERAND_KK,
    OPERAND_NNN,
    OPERAND_I,
    OPERAND_V0,
    OPERAND_DT,
    OPERAND_ST,
    OPERAND_F,
    OPERAND_B,
    OPERAND_K,
    OPERAND_DATA  
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
int disassembler_disassemble(instruction_t *instruction, const uint16_t opcode);
int disassembler_format(char *formatted_instruction, size_t size, const instruction_t *instruction);

const instruction_info_t *disassembler_lookup(uint16_t opcode);
