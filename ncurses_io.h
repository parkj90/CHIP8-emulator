//ncurses_io.h

#pragma once

extern const cpu_io_interface_t ncurses_io_interface;

void ncurses_io_init();
const cpu_io_interface_t *ncurses_io_get_interface();
void ncurses_io_terminate();