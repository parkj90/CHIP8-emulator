//diassembler.h

#include <stdbool.h>

typedef struct opcode opcode_t;
typedef struct romBuffer romBuffer_t;

romBuffer_t *CHIP8_getOpcodes(FILE *game_f);
void romBuffer_free(romBuffer_t *rom);
void disassembler_dump(const romBuffer_t *opcodes);
opcode_t *disassembler_filter(uint16_t opcode);
bool disassembler_print(uint16_t opcode, const char *instruction);
uint16_t disassembler_fetchVar(char type, uint16_t opcode);
