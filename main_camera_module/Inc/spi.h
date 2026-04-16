#ifndef __SPI_H
#define __SPI_H

#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_spi.h"

uint8_t SPI2_ReadWriteByte(uint8_t TxData);
void SPI2_WriteBuffer(uint8_t *data, uint32_t len);

#endif