#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f0xx_hal.h"
#include "nrf24l01p.h"
#include "nrf24l01p_ext.h"
#include "ArduCAM.h"
#include "spi.h"
#include "sccb_bus.h"
#include "stm32f0xx_hal_tim.h"
#include <stdio.h>
#include "camera.h"
#include <string.h>

// Camera CS
#define OUT_CS_PORT GPIOA
#define OUT_CS_PIN  GPIO_PIN_1

#define OUT_CS_LOW()  HAL_GPIO_WritePin(OUT_CS_PORT, OUT_CS_PIN, GPIO_PIN_RESET)
#define OUT_CS_HIGH() HAL_GPIO_WritePin(OUT_CS_PORT, OUT_CS_PIN, GPIO_PIN_SET)

// Image packets (for RF transmission)
#define DATA_PER_PACKET (NRF24L01P_PAYLOAD_LENGTH - 4) // 2 bytes (packet_id) + 2 bytes (total_packets)
typedef struct {
    uint16_t packet_id;
    uint16_t total_packets;
    uint8_t  data[DATA_PER_PACKET];
} __attribute__((packed)) ImagePacket;

#define MAX_JPEG_SIZE 10000 // Max supported JPEG size in bytes

void led_flash_handler(uint32_t *flash_var, uint16_t time_ms, uint16_t pin);
#define RED_PIN GPIO_PIN_6
#define ORANGE_PIN GPIO_PIN_8
#define BLUE_PIN GPIO_PIN_7
#define GREEN_PIN GPIO_PIN_9

void Error_Handler(void);
void SystemClock_Config(void);
void GPIO_Init(void);
void SPI1_Init(void);
void EXTI_Init(void);
void DMA_Init(void);
void SPI2_Init(void);

void I2C1_Init(void);
void uart3_Init(void);
void uart3_write_char(char c);
void uart3_write_string(const char *s);
void spi_send_frame(const uint8_t *data, uint32_t len);
void spi_send_frame_preview(const uint8_t *data, uint32_t len);
void TIM2_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */