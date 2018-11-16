//rombuffer.c

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "rombuffer.h"

rombuffer_t *rombuffer_read(FILE *game_f) {
    if (game_f == NULL) {
        return NULL;
    }

    uint16_t byte_buffer;

    rombuffer_t *rom = malloc(sizeof(rombuffer_t));
    if (rom == NULL) {
        return NULL;
    }

    fseek(game_f, 0, SEEK_END);
    long int end_position = ftell(game_f);
    fseek(game_f, 0, SEEK_SET);
    rom->length = end_position / 2;

    rom->data= malloc(rom->length * sizeof(uint16_t));
    if (rom->data== NULL) {
        return NULL;
    }

    size_t returned_elements; 
    for (size_t i = 0; i < rom->length; i++) {
        if ((returned_elements = fread(&byte_buffer, 2, 1, game_f)) != 1) {
            printf("incorrect # bytes read, returned: %zd\n",
                   returned_elements);
        }
        
        byte_buffer = ntohs(byte_buffer);
        rom->data[i] = byte_buffer;
    }

    return rom;
}

void rombuffer_free(rombuffer_t *rom) {
    if (rom == NULL) {
        return;
    }

    free(rom->data);
    free(rom);
}
