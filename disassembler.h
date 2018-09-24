//diassembler.h

#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct instruction_info instruction_info_t;
typedef struct instruction instruction_t;
typedef enum operand_type operand_type_t;

void disassembler_dump(const rombuffer_t *opcodes);
const instruction_info_t *disassembler_lookup(uint16_t opcode);
void disassembler_disassemble(instruction_t *instruction, uint16_t opcode);
void disassembler_format(char *formatted_instruction, size_t size, const instruction_t *instruction);
