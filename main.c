#include <stdio.h>
#include "rombuffer.h"
#include "disassembler.h"

int main(void) {
    FILE *cnct4 = fopen("../c8games/CONNECT4", "r");

    if (cnct4 == NULL) {
        printf("error opening file\n");
        return -1;
    }

    rombuffer_t *cnct4_opcodes = rombuffer_getOpcodes(cnct4);
    disassembler_dump(cnct4_opcodes);
    rombuffer_free(cnct4_opcodes);
    
    return 0;
}
