//ncurses_io.c

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <curses.h>
#include <pthread.h>
#include <unistd.h>
#include "cpu.h"
#include "ncurses_io.h"

#define FILL_CHAR 35
#define CLEAR_CHAR ' '

static WINDOW *win;
static int keyboard_input;
pthread_mutex_t mutex_ncurses = PTHREAD_MUTEX_INITIALIZER;

enum key_map {
    KEY_MAP_QUIT = 'q',
    KEY_MAP_1 = '6',
    KEY_MAP_2 = '7',
    KEY_MAP_3 = '8',
    KEY_MAP_C = '9',
    KEY_MAP_4 = 'y',
    KEY_MAP_5 = 'u',
    KEY_MAP_6 = 'i',
    KEY_MAP_D = 'o',
    KEY_MAP_7 = 'h',
    KEY_MAP_8 = 'j',
    KEY_MAP_9 = 'k',
    KEY_MAP_E = 'l',
    KEY_MAP_A = 'n',
    KEY_MAP_0 = 'm',
    KEY_MAP_B = ',',
    KEY_MAP_F = '.'
};

static uint16_t ncurses_io_get_keyboard();
static uint8_t ncurses_io_wait_keypress();
static bool ncurses_io_get_pixel(uint8_t x, uint8_t y);    
static void ncurses_io_draw_pixel(uint8_t x, uint8_t y, bool fill);

const cpu_io_interface_t ncurses_io_interface = {
    .get_keyboard = ncurses_io_get_keyboard,
    .wait_keypress = ncurses_io_wait_keypress,
    .get_pixel = ncurses_io_get_pixel,
    .draw_pixel = ncurses_io_draw_pixel
};

void ncurses_io_init() {
    initscr();
    noecho();

    win = newwin(DISPLAY_HEIGHT, DISPLAY_WIDTH, 0, 0);
    nodelay(win, true);
}

void ncurses_io_terminate() {
    endwin();
}

void ncurses_ui_run() {
    while (keyboard_input != KEY_MAP_QUIT) {
        usleep(50000);
        pthread_mutex_lock(&mutex_ncurses);
        keyboard_input = wgetch(win);
        pthread_mutex_unlock(&mutex_ncurses);
    }
}

static uint16_t ncurses_io_get_keyboard() {
    uint16_t hex_keys;

    pthread_mutex_lock(&mutex_ncurses);
    switch (keyboard_input) {
        case ERR:
            hex_keys = 0x00;
            break;
        case KEY_MAP_1:
            hex_keys = 1 << 0x01;
            break;
        case KEY_MAP_2:
            hex_keys = 1 << 0x02;
            break;
        case KEY_MAP_3:
            hex_keys = 1 << 0x03;
            break;
        case KEY_MAP_C:
            hex_keys = 1 << 0x0C;
            break;
        case KEY_MAP_4:
            hex_keys = 1 << 0x04;
            break;
        case KEY_MAP_5:
            hex_keys = 1 << 0x05;
            break;
        case KEY_MAP_6:
            hex_keys = 1 << 0x06;
            break;
        case KEY_MAP_D:
            hex_keys = 1 << 0x0D;
            break;
        case KEY_MAP_7:
            hex_keys = 1 << 0x07;
            break;
        case KEY_MAP_8:
            hex_keys = 1 << 0x08;
            break;
        case KEY_MAP_9:
            hex_keys = 1 << 0x09;
            break;
        case KEY_MAP_E:
            hex_keys = 1 << 0x0E;
            break;
        case KEY_MAP_A:
            hex_keys = 1 << 0x0A;
            break;
        case KEY_MAP_0:
            hex_keys = 1;
            break;
        case KEY_MAP_B:
            hex_keys = 1 << 0x0B;
            break;
        case KEY_MAP_F:
            hex_keys = 1 << 0x0F;
            break;
        default:
            hex_keys = 0x00;
    }
    pthread_mutex_unlock(&mutex_ncurses);

    return hex_keys;
}

static uint8_t ncurses_io_wait_keypress() {
    uint8_t return_value;
    bool unacceptable = true;

    while (unacceptable) {
        unacceptable = false;

        pthread_mutex_lock(&mutex_ncurses);
        switch (keyboard_input) {
            case KEY_MAP_QUIT:
                return_value = 0x00;
                break;
            case ERR:
                unacceptable = true;
                break;
            case KEY_MAP_1:
                return_value = 0x01;
                break;
            case KEY_MAP_2:
                return_value = 0x02;
                break;
            case KEY_MAP_3:
                return_value = 0x03;
                break;
            case KEY_MAP_C:
                return_value = 0x0C;
                break;
            case KEY_MAP_4:
                return_value = 0x04;
                break;
            case KEY_MAP_5:
                return_value = 0x05;
                break;
            case KEY_MAP_6:
                return_value = 0x06;
                break;
            case KEY_MAP_D:
                return_value = 0x0D;
                break;
            case KEY_MAP_7:
                return_value = 0x07;
                break;
            case KEY_MAP_8:
                return_value = 0x08;
                break;
            case KEY_MAP_9:
                return_value = 0x09;
                break;
            case KEY_MAP_E:
                return_value = 0x0E;
                break;
            case KEY_MAP_A:
                return_value = 0x0A;
                break;
            case KEY_MAP_0:
                return_value = 0x00;
                break;
            case KEY_MAP_B:
                return_value = 0x0B;
                break;
            case KEY_MAP_F:
                return_value = 0x0F;
                break;
            default:
                unacceptable = true;
        }
        pthread_mutex_unlock(&mutex_ncurses);
    }

    return return_value;
}

static bool ncurses_io_get_pixel(uint8_t x, uint8_t y) {
    bool return_value;
    pthread_mutex_lock(&mutex_ncurses);

    if (mvwinch(win, y, x) == FILL_CHAR) {
        return_value = true;
    } else {
        return_value = false;
    }

    pthread_mutex_unlock(&mutex_ncurses);

    return return_value;
}

static void ncurses_io_draw_pixel(uint8_t x, uint8_t y, bool fill) {
    pthread_mutex_lock(&mutex_ncurses);
    wmove(win, y, x);
    if (fill) {
        waddch(win, FILL_CHAR);
    } else {
        waddch(win, CLEAR_CHAR);
    }
    wrefresh(win);
    pthread_mutex_unlock(&mutex_ncurses);
}
