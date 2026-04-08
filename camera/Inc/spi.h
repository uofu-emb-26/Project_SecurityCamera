#ifndef __SPI_H
#define __SPI_H

#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_spi.h"

void SPI2_Init(void);
uint8_t SPI2_ReadWriteByte(uint8_t TxData);
void SPI2_WriteBuffer(uint8_t *data, uint32_t len);
void SPI1_Init(void);
void SPI1_WriteBuffer(uint8_t *data, uint32_t len);

#endif