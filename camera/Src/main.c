#include "ArduCAM.h"
#include "spi.h"
#include "sccb_bus.h"
#include "main.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_tim.h"
#include <stdio.h>
#include "camera.h"

I2C_HandleTypeDef hi2c1;
TIM_HandleTypeDef htim2;

#define OUT_CS_PORT GPIOA
#define OUT_CS_PIN  GPIO_PIN_1

#define OUT_CS_LOW()  HAL_GPIO_WritePin(OUT_CS_PORT, OUT_CS_PIN, GPIO_PIN_RESET)
#define OUT_CS_HIGH() HAL_GPIO_WritePin(OUT_CS_PORT, OUT_CS_PIN, GPIO_PIN_SET)

volatile uint8_t capture_request = 0;

void SystemClock_Config(void);
void I2C1_Init(void);
void uart3_Init(void);
void uart3_write_char(char c);
void uart3_write_string(const char *s);
void spi_send_frame(const uint8_t *data, uint32_t len);
void spi_send_frame_preview(const uint8_t *data, uint32_t len);
void OUT_CS_Init(void);
void TIM2_Init(void);
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    I2C1_Init();
    uart3_Init();
    SPI2_Init();
    SPI1_Init();
    OUT_CS_Init();
    camera_init(&hi2c1);
    TIM2_Init();

    while (1)
    {
        if (capture_request){
            capture_request = 0;
            uart3_write_string("timer fired\r\n");

            uint32_t len = camera_capture_frame();
            char msg[40];
            snprintf(msg, sizeof(msg), "len=%lu\r\n", len);
            uart3_write_string(msg);

            if (len > 0)
            {
                const uint8_t *jpeg = camera_get_buffer();
                spi_send_frame(jpeg, len);
            }
        }
    }
}

void uart3_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
    RCC->AHBENR  |= RCC_AHBENR_GPIOCEN;

    GPIOC->MODER &= ~((3U<<(10*2)) | (3U<<(11*2)));
    GPIOC->MODER |=  ((2U<<(10*2)) | (2U<<(11*2)));

    GPIOC->AFR[1] &= ~((0xF << ((10-8)*4)) | (0xF << ((11-8)*4)));
    GPIOC->AFR[1] |=  ((1   << ((10-8)*4)) | (1   << ((11-8)*4)));

    USART3->BRR = (uint16_t)(48000000 / 1500000);
    USART3->CR1 = USART_CR1_TE | USART_CR1_RE;
    USART3->CR1 |= USART_CR1_UE;
}

void uart3_write_char(char c)
{
    while ((USART3->ISR & USART_ISR_TXE) == 0) {}
    USART3->TDR = (uint8_t)c;
}

void uart3_write_string(const char *s)
{
    while (*s) uart3_write_char(*s++);
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48;
    RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI48;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1);
}

void I2C1_Init(void)
{
    __HAL_RCC_I2C1_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF1_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x10805E89;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    HAL_I2C_Init(&hi2c1);
}

void spi_send_frame(const uint8_t *data, uint32_t len)
{
    uint8_t start[2] = {0xAA, 0x55};
    uint8_t len_bytes[4];

    len_bytes[0] = (uint8_t)(len >> 0);
    len_bytes[1] = (uint8_t)(len >> 8);
    len_bytes[2] = (uint8_t)(len >> 16);
    len_bytes[3] = (uint8_t)(len >> 24);

    OUT_CS_LOW();

    SPI1_WriteBuffer(start, 2);
    SPI1_WriteBuffer(len_bytes, 4);
    SPI1_WriteBuffer((uint8_t*)data, len);

    HAL_Delay(50); 
    OUT_CS_HIGH();
}

void OUT_CS_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = OUT_CS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(OUT_CS_PORT, &GPIO_InitStruct);

    OUT_CS_HIGH();
}

void TIM2_Init(void){
    __HAL_RCC_TIM2_CLK_ENABLE();

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 47999;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 3999;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    HAL_TIM_Base_Init(&htim2);

    HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);

    HAL_TIM_Base_Start_IT(&htim2);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        capture_request = 1;
    }
}

void spi_send_test(void)
{
    uint8_t test[] = {
        0xAA, 0x55, 0x11, 0x22,
        0x33, 0x44, 0x55, 0x66,
        0x77, 0x88, 0x99, 0xAA
    };

    OUT_CS_LOW();
    HAL_Delay(1);

    SPI1_WriteBuffer(test, sizeof(test));

    HAL_Delay(1);
    OUT_CS_HIGH();
}