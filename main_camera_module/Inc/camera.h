#ifndef __CAMERA_H
#define __CAMERA_H

#include "stm32f0xx_hal.h"
#include "ArduCAM.h"
#include "spi.h"
#include "sccb_bus.h"
#include "main.h"

// Maximum frame buffer size - enough for 320x240 JPEG
#define FRAME_BUFFER_SIZE 8192

// Call this once at startup before capturing
void camera_init(I2C_HandleTypeDef *hi2c);

// Capture one JPEG frame into the internal buffer
// Returns number of bytes captured, 0 on error
uint32_t camera_capture_frame(void);

// Get pointer to the raw JPEG bytes after capture
// Valid until next call to camera_capture_frame()
const uint8_t* camera_get_buffer(void);

// Get the size of the last captured frame in bytes
uint32_t camera_get_frame_size(void);

#endif