#ifndef __SPI_H
#define __SPI_H

#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_spi.h"

void SPI1_Init(void);
uint8_t SPI1_ReadWriteByte(uint8_t TxData);

#endif