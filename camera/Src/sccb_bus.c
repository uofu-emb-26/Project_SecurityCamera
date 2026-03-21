#include "sccb_bus.h"

static I2C_HandleTypeDef *_hi2c;

void sccb_bus_init(I2C_HandleTypeDef *hi2c)
{
    _hi2c = hi2c;
}

void sccb_bus_start(void)      {}
void sccb_bus_stop(void)       {}
void sccb_bus_send_noack(void) {}
void sccb_bus_send_ack(void)   {}
uint8_t sccb_bus_write_byte(uint8_t data) { (void)data; return 1; }
uint8_t sccb_bus_read_byte(void) { return 0; }

HAL_StatusTypeDef sccb_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t dat)
{
    uint8_t buf[2] = { reg, dat };
    return HAL_I2C_Master_Transmit(_hi2c, dev_addr, buf, 2, HAL_MAX_DELAY);
}

HAL_StatusTypeDef sccb_read_reg(uint8_t dev_addr, uint8_t reg, uint8_t *dat)
{
    if (HAL_I2C_Master_Transmit(_hi2c, dev_addr, &reg, 1, HAL_MAX_DELAY) != HAL_OK)
        return HAL_ERROR;
    return HAL_I2C_Master_Receive(_hi2c, dev_addr | 0x01, dat, 1, HAL_MAX_DELAY);
}