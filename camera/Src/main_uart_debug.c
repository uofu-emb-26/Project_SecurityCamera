#include "ArduCAM.h"
#include "spi.h"
#include "sccb_bus.h"
#include "main.h"
#include "stm32f0xx_hal.h"
#include <stdio.h>

#define OV2640_I2C_ADDR  0x60

I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart3;
TIM_HandleTypeDef htim2;

void SystemClock_Config(void);
void I2C1_Init(void);
void uart3_Init(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    SPI2_Init();
    I2C1_Init();
    uart3_Init();

    sccb_bus_init(&hi2c1);
    sensor_addr = OV2640_I2C_ADDR;

    ArduCAM_CS_init();
    ArduCAM_Init(OV2640);
    HAL_Delay(1);

    while(1)
    {
        // Start capture
        flush_fifo();
        clear_fifo_flag();
        start_capture();

        // Wait for capture with timeout
        uint32_t timeout = HAL_GetTick();
        while (!(get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK)))
        {
            if (HAL_GetTick() - timeout > 3000)
            {
                uart3_write_string("ERR:TIMEOUT\r\n");
                break;
            }
        }

        // Read length
        uint32_t len = read_fifo_length();
        if (len == 0 || len >= MAX_FIFO_SIZE)
        {
            uart3_write_string("ERR:BADLEN\r\n");
            continue;
        }

        // Send frame
        uart3_write_string("IMG:START\r\n");
        uart3_write_char((len >>  0) & 0xFF);
        uart3_write_char((len >>  8) & 0xFF);
        uart3_write_char((len >> 16) & 0xFF);
        uart3_write_char((len >> 24) & 0xFF);

        CS_LOW();
        set_fifo_burst();

        static uint8_t buf[128];
        uint32_t remaining = len;

        uint8_t prev = 0;
        while (remaining > 0)
        {
            // uint32_t chunk = (remaining > sizeof(buf)) ? sizeof(buf) : remaining;
            // for (uint32_t i = 0; i < chunk; i++)
            //     buf[i] = SPI2_ReadWriteByte(0x00);
            // for (uint32_t i = 0; i < chunk; i++)
            //     uart3_write_char((char)buf[i]);
            // remaining -= chunk;
            uint8_t b = SPI2_ReadWriteByte(0x00);
            uart3_write_char((char)b);
            remaining--;
        }

        CS_HIGH();
        uart3_write_string("IMG:END\r\n");

        // Small delay between frames
        HAL_Delay(100);
    }
}

void uart3_Init(void)
{
    // Enable clocks
    RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
    RCC->AHBENR  |= RCC_AHBENR_GPIOCEN;

    // PC10=TX, PC11=RX alternate function mode
    GPIOC->MODER &= ~((3U<<(10*2)) | (3U<<(11*2)));
    GPIOC->MODER |=  ((2U<<(10*2)) | (2U<<(11*2)));

    // AF1 = USART3 on PC10/PC11
    GPIOC->AFR[1] &= ~((0xF << ((10-8)*4)) | (0xF << ((11-8)*4)));
    GPIOC->AFR[1] |=  ((1   << ((10-8)*4)) | (1   << ((11-8)*4)));

    // Baud rate: 48MHz / 115200
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

void I2C1_Init(void)
{
    __HAL_RCC_I2C1_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7; // PB6=SCL, PB7=SDA
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF1_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x10805E89; // 100kHz @ 48MHz HSI48
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    HAL_I2C_Init(&hi2c1);
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