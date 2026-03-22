// MIT License
//
// Copyright (c) 2020 ardnew
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Source: https://github.com/ardnew/ILI9341-STM32-HAL/tree/master

#ifndef __ILI9341_GFX_H
#define __ILI9341_GFX_H

#ifdef __cplusplus
extern "C" {
#endif

// ----------------------------------------------------------------- includes --

#include <stdint.h>

#include "ili9341.h"
#include "ili9341_font.h"

// ------------------------------------------------------------------ defines --

/* nothing */

// ------------------------------------------------------------------- macros --

#define __ILI9341_COLOR565(r,g,b) \
    ( (((r) & 0xF8) << 8) |  \
      (((g) & 0xFC) << 3) |  \
      (((b) & 0xF8) >> 3) )

#define __ILI9341_COLOR565_RGB(rgb) \
    __ILI9341_COLOR565((rgb).red, (rgb).green, (rgb).blue)

#define __ILI9341_RGB(r,g,b) \
    (ili9341_color_rgb_t){ .red = (r), .green = (g), .blue = (b) }

// ----------------------------------------------------------- exported types --

typedef uint16_t ili9341_color_t;

typedef struct
{
  int16_t red;
  int16_t green;
  int16_t blue;
}
ili9341_color_rgb_t;

// TODO:
typedef enum
{
  iwwNONE = -1,
  iwwTruncate, // = 0
  iwwCharWrap, // = 1
  iwwWordWrap, // = 2
  iwwCOUNT     // = 3
}
ili9341_word_wrap_t;

typedef struct
{
  ili9341_font_t const *font;
  ili9341_color_t fg_color;
  ili9341_color_t bg_color;
  uint16_t origin_x;
  uint16_t origin_y;
}
ili9341_text_attr_t;

// ------------------------------------------------------- exported variables --

extern ili9341_color_t const ILI9341_BLACK;
extern ili9341_color_t const ILI9341_NAVY;
extern ili9341_color_t const ILI9341_DARKGREEN;
extern ili9341_color_t const ILI9341_DARKCYAN;
extern ili9341_color_t const ILI9341_MAROON;
extern ili9341_color_t const ILI9341_PURPLE;
extern ili9341_color_t const ILI9341_OLIVE;
extern ili9341_color_t const ILI9341_LIGHTGREY;
extern ili9341_color_t const ILI9341_DARKGREY;
extern ili9341_color_t const ILI9341_BLUE;
extern ili9341_color_t const ILI9341_GREEN;
extern ili9341_color_t const ILI9341_CYAN;
extern ili9341_color_t const ILI9341_RED;
extern ili9341_color_t const ILI9341_MAGENTA;
extern ili9341_color_t const ILI9341_YELLOW;
extern ili9341_color_t const ILI9341_WHITE;
extern ili9341_color_t const ILI9341_ORANGE;
extern ili9341_color_t const ILI9341_GREENYELLOW;
extern ili9341_color_t const ILI9341_PINK;

// ------------------------------------------------------- exported functions --

ili9341_color_rgb_t ili9341_rgb_wheel(uint8_t *pos);
ili9341_color_t ili9341_color_wheel(uint8_t *pos);

void ili9341_spi_tft_set_address_rect(ili9341_t *lcd,
    uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

void ili9341_transmit_wait(ili9341_t *lcd);
void ili9341_transmit_color(ili9341_t *lcd, uint32_t size,
    uint16_t color[]/* already byte-swapped (LE) */, ili9341_bool_t wait);

void ili9341_draw_pixel(ili9341_t *lcd, ili9341_color_t color,
    int16_t x, int16_t y);

void ili9341_draw_line(ili9341_t *lcd, ili9341_color_t color,
    int16_t x0, int16_t y0, int16_t x1, int16_t y1);

void ili9341_draw_rect(ili9341_t *lcd, ili9341_color_t color,
    int16_t x, int16_t y, uint16_t w, uint16_t h);
void ili9341_fill_rect(ili9341_t *lcd, ili9341_color_t color,
    int16_t x, int16_t y, uint16_t w, uint16_t h);

void ili9341_draw_circle(ili9341_t *lcd, ili9341_color_t color,
    int16_t x, int16_t y, int16_t r);
void ili9341_fill_circle(ili9341_t *lcd, ili9341_color_t color,
    int16_t x, int16_t y, int16_t r);

void ili9341_fill_screen(ili9341_t *lcd, ili9341_color_t color);

void ili9341_draw_bitmap_1b(ili9341_t *lcd,
    ili9341_color_t fg_color, ili9341_color_t bg_color,
    int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t *bmp);

void ili9341_draw_char(ili9341_t *lcd, ili9341_text_attr_t attr, char ch);
void ili9341_draw_string(ili9341_t *lcd, ili9341_text_attr_t attr, char str[]);

#ifdef __cplusplus
}
#endif

#endif /* __ILI9341_GFX_H */
