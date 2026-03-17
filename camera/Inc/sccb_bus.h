#ifndef __SCCB_BUS_H
#define __SCCB_BUS_H

#include "stm32f0xx_hal.h"

void sccb_bus_init(I2C_HandleTypeDef *hi2c);
void sccb_bus_start(void);
void sccb_bus_stop(void);
uint8_t sccb_bus_write_byte(uint8_t data);
uint8_t sccb_bus_read_byte(void);
void sccb_bus_send_noack(void);
void sccb_bus_send_ack(void);

#endif