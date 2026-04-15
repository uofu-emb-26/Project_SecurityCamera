#ifndef __IMAGES_H
#define __IMAGES_H

#include <stdint.h>

// __REV16()
#include "stm32f0xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const uint16_t red_16x16[256];
extern uint8_t dirt_16x16[512];

void ili9341_array_endian_swap(uint16_t *buffer16, uint32_t num_pixels);

#ifdef __cplusplus
}
#endif

#endif /* __IMAGES_H */