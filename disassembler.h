//diassembler.h

#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct instruction instruction_t;

void disassembler_dump(const rombuffer_t *opcodes);
const instruction_t *disassembler_lookup(uint16_t opcode);
void disassembler_fetch_operands(uint16_t *operands, uint16_t opcode, const instruction_t *instruction);
void disassembler_format_operands(char *formatted_operands, size_t size, const uint16_t *operands, const instruction_t *instruction);
void disassembler_format(char *formatted_instruction, size_t size, const char *mnemonic, const char *formatted_operands);
