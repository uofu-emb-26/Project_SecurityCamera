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

#define TFT_RST_Pin GPIO_PIN_8
#define TFT_RST_GPIO_Port GPIOA
#define TFT_CS_Pin GPIO_PIN_9
#define TFT_CS_GPIO_Port GPIOA
#define TFT_DC_Pin GPIO_PIN_10
#define TFT_DC_GPIO_Port GPIOA

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