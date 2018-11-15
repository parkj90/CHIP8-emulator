//sdl_io.h

#pragma once

extern const cpu_io_interface_t sdl_io_interface;

void sdl_io_init(const char *game_title);
void sdl_io_terminate();
void sdl_ui_run();
