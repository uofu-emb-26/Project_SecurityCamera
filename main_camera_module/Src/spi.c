#include "spi.h"

extern SPI_HandleTypeDef hspi2;

uint8_t SPI2_ReadWriteByte(uint8_t TxData)
{
    uint8_t RxData = 0;
    HAL_SPI_TransmitReceive(&hspi2, &TxData, &RxData, 1, 100);
    return RxData;
}

void SPI2_WriteBuffer(uint8_t *data, uint32_t len)
{
    HAL_SPI_Transmit(&hspi2, data, len, HAL_MAX_DELAY);
}
