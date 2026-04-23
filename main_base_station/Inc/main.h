#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f0xx_hal.h"
#include "ili9341.h"
#include "images.h"
#include "ili9341_ext.h"
#include "ili9341_gfx.h"
#include "jpeg_decode.h"
#include "nrf24l01p.h"
#include "nrf24l01p_ext.h"
#include <string.h>
#include <stdio.h>

#define TFT_RST_Pin GPIO_PIN_8
#define TFT_RST_GPIO_Port GPIOA
#define TFT_CS_Pin GPIO_PIN_9
#define TFT_CS_GPIO_Port GPIOA
#define TFT_DC_Pin GPIO_PIN_10
#define TFT_DC_GPIO_Port GPIOA

#define RED_PIN GPIO_PIN_6
#define ORANGE_PIN GPIO_PIN_8
#define BLUE_PIN GPIO_PIN_7
#define GREEN_PIN GPIO_PIN_9

// Image packets (for RF transmission)
#define DATA_PER_PACKET (NRF24L01P_PAYLOAD_LENGTH - 4) // 2 bytes (packet_id) + 2 bytes (total_packets)
typedef struct {
    uint16_t packet_id;
    uint16_t total_packets;
    uint8_t  data[DATA_PER_PACKET];
} __attribute__((packed)) ImagePacket;

#define MAX_JPEG_SIZE 10000 // Max supported JPEG size in bytes
#define MAX_PACKETS ((MAX_JPEG_SIZE + DATA_PER_PACKET - 1) / DATA_PER_PACKET)

void Error_Handler(void);
void SystemClock_Config(void);
void GPIO_Init(void);
void SPI1_Init(void);
void EXTI_Init(void);
void DMA_Init(void);
void SPI2_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */