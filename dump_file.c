#include <stdio.h>
#include <stdint.h>
#include <assert.h>

int main(void) {
    FILE *new_file = fopen("./c8games/CONNECT4", "r");

    int c;
    uint16_t d;
    printf("%d\n", sizeof(c));
    int count = 0;
    while (EOF != (c = fgetc(new_file))) {
        d = c;
        printf("%x ", d);
        count++;
        if (count == 16) {
            printf("\n");
            count = 0;
        }
    }

    assert(fclose(new_file) == 0);
    printf("\ndone\n");
}
