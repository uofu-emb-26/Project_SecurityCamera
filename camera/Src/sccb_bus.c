#include "sccb_bus.h"

static I2C_HandleTypeDef *_hi2c;
static uint8_t _dev_addr;

void sccb_bus_init(I2C_HandleTypeDef *hi2c)
{
    _hi2c = hi2c;
}

uint8_t sccb_bus_write_byte(uint8_t data)
{
    // handled by HAL I2C directly
    return 1;
}

uint8_t sccb_bus_read_byte(void)
{
    return 0;
}

void sccb_bus_start(void) {}
void sccb_bus_stop(void) {}
void sccb_bus_send_noack(void) {}
void sccb_bus_send_ack(void) {}