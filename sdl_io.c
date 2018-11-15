//sdl_io.c

#include <stdbool.h>
#include <SDL2/SDL.h>
#include <pthread.h>
#include <unistd.h>
#include "cpu.h"
#include "sdl_io.h"

#define PIXEL_WIDTH 15

static SDL_Window *win;
static SDL_Renderer *renderer;
static pthread_mutex_t mutex_sdl = PTHREAD_MUTEX_INITIALIZER;
static const uint8_t *keyboard_state;
static uint32_t current_wait_key;
static bool pixel_map[DISPLAY_HEIGHT][DISPLAY_WIDTH];

enum key_map {
    KEY_MAP_1 = SDL_SCANCODE_6,
    KEY_MAP_2 = SDL_SCANCODE_7,
    KEY_MAP_3 = SDL_SCANCODE_8,
    KEY_MAP_C = SDL_SCANCODE_9,
    KEY_MAP_4 = SDL_SCANCODE_Y,
    KEY_MAP_5 = SDL_SCANCODE_U,
    KEY_MAP_6 = SDL_SCANCODE_I,
    KEY_MAP_D = SDL_SCANCODE_O,
    KEY_MAP_7 = SDL_SCANCODE_H,
    KEY_MAP_8 = SDL_SCANCODE_J,
    KEY_MAP_9 = SDL_SCANCODE_K,
    KEY_MAP_E = SDL_SCANCODE_L,
    KEY_MAP_A = SDL_SCANCODE_N,
    KEY_MAP_0 = SDL_SCANCODE_M,
    KEY_MAP_B = SDL_SCANCODE_COMMA,
    KEY_MAP_F = SDL_SCANCODE_PERIOD
};

enum key_wait {
    KEY_WAIT_1 = SDLK_6,
    KEY_WAIT_2 = SDLK_7,
    KEY_WAIT_3 = SDLK_8,
    KEY_WAIT_C = SDLK_9,
    KEY_WAIT_4 = SDLK_y,
    KEY_WAIT_5 = SDLK_u,
    KEY_WAIT_6 = SDLK_i,
    KEY_WAIT_D = SDLK_o,
    KEY_WAIT_7 = SDLK_h,
    KEY_WAIT_8 = SDLK_j,
    KEY_WAIT_9 = SDLK_k,
    KEY_WAIT_E = SDLK_l,
    KEY_WAIT_A = SDLK_n,
    KEY_WAIT_0 = SDLK_m,
    KEY_WAIT_B = SDLK_COMMA,
    KEY_WAIT_F = SDLK_PERIOD
};

static uint16_t sdl_io_get_keyboard();
static uint8_t sdl_io_wait_keypress();
static bool sdl_io_get_pixel(uint8_t x, uint8_t y);
static void sdl_io_draw_pixel(uint8_t x, uint8_t y, bool fill);

const cpu_io_interface_t sdl_io_interface = {
    .get_keyboard = sdl_io_get_keyboard, 
    .wait_keypress = sdl_io_wait_keypress,
    .get_pixel = sdl_io_get_pixel,
    .draw_pixel = sdl_io_draw_pixel
};

void sdl_io_init(const char *game_title) {
    SDL_Init(SDL_INIT_VIDEO);

    win = SDL_CreateWindow(
        game_title,
        SDL_WINDOWPOS_UNDEFINED, 
        SDL_WINDOWPOS_UNDEFINED,
        DISPLAY_WIDTH * PIXEL_WIDTH,
        DISPLAY_HEIGHT * PIXEL_WIDTH,
        0
    );
    if (win == NULL) {
        printf("Error creating window: %s\n", SDL_GetError());
        return;
    }

    renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE);
    if (renderer == NULL) {
        printf("Error creating renderer: %s\n", SDL_GetError());
    }

    //clear game screen to black
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}

void sdl_io_terminate() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
}

void sdl_ui_run() {
    bool quit = false;
    SDL_Event event;

    while (!quit) {
        pthread_mutex_lock(&mutex_sdl);
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
                current_wait_key = KEY_WAIT_0;
            }

            if (event.type == SDL_KEYDOWN) {
                current_wait_key = event.key.keysym.sym;
            }
        }
        
        keyboard_state = SDL_GetKeyboardState(NULL);

        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderClear(renderer);

        SDL_Rect square_pixel = {
            0,
            0,
            PIXEL_WIDTH,
            PIXEL_WIDTH
        };

        for (int i = 0; i < DISPLAY_HEIGHT; i++) {
            for (int j = 0; j < DISPLAY_WIDTH; j++) {
                square_pixel.x = j * PIXEL_WIDTH;
                square_pixel.y = i * PIXEL_WIDTH;

                if (pixel_map[i][j]) {
                    SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0x00);
                } else {
                    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
                }

                SDL_RenderFillRect(renderer, &square_pixel);
            }
        }

        SDL_RenderPresent(renderer);
        pthread_mutex_unlock(&mutex_sdl);
        usleep(10000);
    }
}

static uint16_t sdl_io_get_keyboard() {
    uint16_t hex_keys = 0;

    pthread_mutex_lock(&mutex_sdl);
    if (keyboard_state[KEY_MAP_1]) {
        hex_keys |= 1 << 0x01;
    }
    if (keyboard_state[KEY_MAP_2]) {
        hex_keys |= 1 << 0x02;
    }
    if (keyboard_state[KEY_MAP_3]) {
        hex_keys |= 1 << 0x03;
    }
    if (keyboard_state[KEY_MAP_C]) {
        hex_keys |= 1 << 0x0C;
    }
    if (keyboard_state[KEY_MAP_4]) {
        hex_keys |= 1 << 0x04;
    }
    if (keyboard_state[KEY_MAP_5]) {
        hex_keys |= 1 << 0x05;
    }
    if (keyboard_state[KEY_MAP_6]) {
        hex_keys |= 1 << 0x06;
    }
    if (keyboard_state[KEY_MAP_D]) {
        hex_keys |= 1 << 0x0D;
    }
    if (keyboard_state[KEY_MAP_7]) {
        hex_keys |= 1 << 0x07;
    }
    if (keyboard_state[KEY_MAP_8]) {
        hex_keys |= 1 << 0x08;
    }
    if (keyboard_state[KEY_MAP_9]) {
        hex_keys |= 1 << 0x09;
    }
    if (keyboard_state[KEY_MAP_E]) {
        hex_keys |= 1 << 0x0E;
    }
    if (keyboard_state[KEY_MAP_A]) {
        hex_keys |= 1 << 0x0A;
    }
    if (keyboard_state[KEY_MAP_0]) {
        hex_keys |= 1;
    }
    if (keyboard_state[KEY_MAP_B]) {
        hex_keys |= 1 << 0x0B;
    }
    if (keyboard_state[KEY_MAP_F]) {
        hex_keys |= 1 << 0x0F;
    }
    pthread_mutex_unlock(&mutex_sdl);

    return hex_keys;
}

static uint8_t sdl_io_wait_keypress() {
    uint8_t return_value;
    bool unacceptable = true;

    while (unacceptable) {
        unacceptable = false;

        pthread_mutex_lock(&mutex_sdl);
        switch (current_wait_key) {
            case KEY_WAIT_1:
                return_value = 0x01;
                break;
            case KEY_WAIT_2:
                return_value = 0x02;
                break;
            case KEY_WAIT_3:
                return_value = 0x03;
                break;
            case KEY_WAIT_C:
                return_value = 0x0C;
                break;
            case KEY_WAIT_4:
                return_value = 0x04;
                break;
            case KEY_WAIT_5:
                return_value = 0x05;
                break;
            case KEY_WAIT_6:
                return_value = 0x06;
                break;
            case KEY_WAIT_D:
                return_value = 0x0D;
                break;
            case KEY_WAIT_7:
                return_value = 0x07;
                break;
            case KEY_WAIT_8:
                return_value = 0x08;
                break;
            case KEY_WAIT_9:
                return_value = 0x09;
                break;
            case KEY_WAIT_E:
                return_value = 0x0E;
                break;
            case KEY_WAIT_A:
                return_value = 0x0A;
                break;
            case KEY_WAIT_0:
                return_value = 0x00;
                break;
            case KEY_WAIT_B:
                return_value = 0x0B;
                break;
            case KEY_WAIT_F:
                return_value = 0x0F;
                break;
            default:
                unacceptable = true;
        }
        current_wait_key = SDLK_UNKNOWN;
        pthread_mutex_unlock(&mutex_sdl);
    }

    return return_value;
}

static bool sdl_io_get_pixel(uint8_t x, uint8_t y) {
    bool return_value;

    pthread_mutex_lock(&mutex_sdl);
    if (pixel_map[y][x]) {
        return_value = true;
    } else {
        return_value = false; 
    }
    pthread_mutex_unlock(&mutex_sdl);

    return return_value;
}

static void sdl_io_draw_pixel(uint8_t x, uint8_t y, bool fill) {
    pthread_mutex_lock(&mutex_sdl);
        pixel_map[y][x] = fill;
    pthread_mutex_unlock(&mutex_sdl);
}
