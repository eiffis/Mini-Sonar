#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

void display_init(void);
void display_show_text(const char *text, uint16_t y);

#endif
