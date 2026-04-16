#ifndef __SPI_H
#define __SPI_H

#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_spi.h"

extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;
extern DMA_HandleTypeDef hdma_spi1_tx;

void SPI1_GPIO_Init(void);
void SPI1_DMA_Init(void);
void SPI1_Init(void);
void SPI1_WriteBuffer(uint8_t *data, uint32_t len);

void SPI2_Init(void);
uint8_t SPI2_ReadWriteByte(uint8_t TxData);
void SPI2_WriteBuffer(uint8_t *data, uint32_t len);

#endif