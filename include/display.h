#ifndef DISPLAY_H
#define DISPLAY_H

#include "fonts.h"
#include <stdint.h>

#define PANEL_WIDTH 240U
#define PANEL_HEIGHT 320U

#define DISPLAY_WIDTH PANEL_HEIGHT
#define DISPLAY_HEIGHT PANEL_WIDTH

#define DISPLAY_LANDSCAPE_FLIP 1

#define DISPLAY_FRONT ((volatile uint16_t *)0xD0000000U)
#define DISPLAY_BACK                                                           \
  ((volatile uint16_t *)(0xD0000000U + DISPLAY_WIDTH * DISPLAY_HEIGHT * 2U))

#define COLOR_BLACK 0x0000U
#define COLOR_WHITE 0xFFFFU
#define COLOR_RED 0xF800U
#define COLOR_GREEN 0x07E0U
#define COLOR_DARK_GREEN 0x0320U

static inline uint32_t display_index(uint32_t x, uint32_t y) {
#if DISPLAY_LANDSCAPE_FLIP
  return (PANEL_HEIGHT - 1U - x) * PANEL_WIDTH + y;
#else
  return x * PANEL_WIDTH + (PANEL_WIDTH - 1U - y);
#endif
}

void display_init(void);
void display_fill(volatile uint16_t *buffer, uint16_t color);
void display_draw_text(volatile uint16_t *buffer, const sFONT *font, uint16_t x,
                       uint16_t y, const char *text, uint16_t color,
                       uint16_t background);
void display_show_text(const char *text, uint16_t y);

#endif
