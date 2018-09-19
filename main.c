#include <stdio.h>
#include "rombuffer.h"
#include "disassembler.h"

int main(void) {
    FILE *cnct4 = fopen("../c8games/CONNECT4", "r");
    if (cnct4 == NULL) {
        perror("Error: ");
        return -1;
    }

    rombuffer_t *cnct4_opcodes = rombuffer_read(cnct4);
    if (cnct4_opcodes == NULL) {
        perror("Error: ");
        return -1;
    }

    disassembler_dump(cnct4_opcodes);
    rombuffer_free(cnct4_opcodes);
    
    return 0;
}
