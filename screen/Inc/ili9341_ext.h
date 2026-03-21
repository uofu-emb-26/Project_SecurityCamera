// Created by Zachary Ward to extend the ILI9341 TFT driver to write raw buffer
// data using DMA.

#ifndef __ILI9341_EXT_H
#define __ILI9341_EXT_H

#include <stdint.h>
#include "ili9341.h"
#include "ili9341_gfx.h"

// The display uses RGB565 (with 16 color bits per pixel)
#define BYTES_PER_PIXEL 2

#ifdef __cplusplus
extern "C" {
#endif

void ili9341_draw_buffer(ili9341_t *lcd, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *buffer);

#ifdef __cplusplus
}
#endif

#endif /* __ILI9341_EXT_H */