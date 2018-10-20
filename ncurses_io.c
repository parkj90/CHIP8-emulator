//ncurses_io.c

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <curses.h>
#include "cpu.h"
#include "ncurses_io.h"

#define FILL_CHAR 35
#define CLEAR_CHAR ' '


enum key_map {
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

static int ncurses_io_get_keyboard();
static uint8_t ncurses_io_wait_keypress();
static bool ncurses_io_get_pixel(uint8_t x, uint8_t y);    
static void ncurses_io_draw_pixel(uint8_t x, uint8_t y, bool fill);

const cpu_io_interface_t ncurses_io_interface = {
    .get_keyboard = ncurses_io_get_keyboard,
    .wait_keypress = ncurses_io_wait_keypress,
    .get_pixel = ncurses_io_get_pixel,
    .draw_pixel = ncurses_io_draw_pixel
};

static WINDOW *win;

void ncurses_io_init() {
    initscr();
    noecho();

    win = newwin(DISPLAY_HEIGHT, DISPLAY_WIDTH, 0, 0);
    nodelay(win, true);
}

void ncurses_io_terminate() {
    endwin();
}

static int ncurses_io_get_keyboard() {
    switch (wgetch(win)) {
        case ERR:
            return -1;
        case KEY_MAP_1:
            return 0x01;
        case KEY_MAP_2:
            return 0x02;
        case KEY_MAP_3:
            return 0x03;
        case KEY_MAP_C:
            return 0x0C;
        case KEY_MAP_4:
            return 0x04;
        case KEY_MAP_5:
            return 0x05;
        case KEY_MAP_6:
            return 0x06;
        case KEY_MAP_D:
            return 0x0D;
        case KEY_MAP_7:
            return 0x07;
        case KEY_MAP_8:
            return 0x08;
        case KEY_MAP_9:
            return 0x09;
        case KEY_MAP_E:
            return 0x0E;
        case KEY_MAP_A:
            return 0x0A;
        case KEY_MAP_0:
            return 0x00;
        case KEY_MAP_B:
            return 0x0B;
        case KEY_MAP_F:
            return 0x0F;
    }

    return -1;
}

static uint8_t ncurses_io_wait_keypress() {
    int stdinput_char;
    uint8_t return_value;
    bool unacceptable = true;
    nodelay(win, false);

    while (unacceptable) {
        stdinput_char = wgetch(win);
        unacceptable = false;
        switch (stdinput_char) {
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
    }

    nodelay(win, true);
    return return_value;
}

static bool ncurses_io_get_pixel(uint8_t x, uint8_t y) {
    if (mvwinch(win, y, x) == FILL_CHAR) {
        return true;
    } else {
        return false;
    }
}

static void ncurses_io_draw_pixel(uint8_t x, uint8_t y, bool fill) {
    wmove(win, y, x);
    if (fill) {
        waddch(win, FILL_CHAR);
    } else {
        waddch(win, CLEAR_CHAR);
    }
    wrefresh(win);
}
