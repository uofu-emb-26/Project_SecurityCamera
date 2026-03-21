#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f0xx_hal.h"
#include "ili9341.h"

#define TFT_RST_Pin GPIO_PIN_8
#define TFT_RST_GPIO_Port GPIOA
#define TFT_CS_Pin GPIO_PIN_9
#define TFT_CS_GPIO_Port GPIOA
#define TFT_DC_Pin GPIO_PIN_10
#define TFT_DC_GPIO_Port GPIOA

void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */