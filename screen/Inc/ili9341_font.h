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


#ifndef __ILI9341_FONT_H
#define __ILI9341_FONT_H

#ifdef __cplusplus
extern "C" {
#endif

// ----------------------------------------------------------------- includes --

#include <stdint.h>

// ------------------------------------------------------------------ defines --

/* nothing */

// ------------------------------------------------------------------- macros --

/* nothing */

// ----------------------------------------------------------- exported types --

typedef struct
{
    const uint8_t width;
    const uint8_t height;
    const uint16_t glyph[];
}
ili9341_font_t;

// ------------------------------------------------------- exported variables --

extern ili9341_font_t const ili9341_font_7x10;
extern ili9341_font_t const ili9341_font_11x18;
extern ili9341_font_t const ili9341_font_16x26;

// ------------------------------------------------------- exported functions --

uint8_t glyph_index(unsigned char glyph);

#ifdef __cplusplus
}
#endif

#endif /* __ILI9341_FONT_H */