#pragma once
#include "../includes/string.hpp"

bool create_window(char *title, int width, int height, unsigned long long *handle);

void print_error(const char *message);

void log_serial(const char *message);

void append_debug_log(const char *message);

void refresh_window(unsigned long long handle);

void wait_window();

bool delete_window(unsigned long long handle);

void draw_pixel(unsigned long long handle, int x, int y, unsigned int color);

void draw_text(unsigned long long handle, int x, int y, unsigned int color, unsigned int size, const stardustui::string& text);
