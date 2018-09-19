//diassembler.h

#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef struct instruction instruction_t;

void disassembler_dump(const rombuffer_t *opcodes);
const instruction_t *disassembler_lookup(uint16_t opcode);
bool disassembler_print(uint16_t opcode, const instruction_t *instruction);
uint16_t disassembler_fetch_operand(uint16_t opcode, uint16_t operand_masks);
