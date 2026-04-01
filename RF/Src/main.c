#include "main.h"
#include "nrf24l01p.h"
#include <string.h>
#include <stdio.h>

SPI_HandleTypeDef hspi2;
UART_HandleTypeDef huart3;

void SystemClock_Config(void);

void SPI2_Init(void) {
    __HAL_RCC_SPI2_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    // SCK=PB13, MISO=PB14
    GPIO_InitTypeDef initStr_SPI2B = {GPIO_PIN_13 | GPIO_PIN_14,
                                      GPIO_MODE_AF_PP,
                                      GPIO_NOPULL,
                                      GPIO_SPEED_FREQ_HIGH,
                                      GPIO_AF0_SPI2};
    HAL_GPIO_Init(GPIOB, &initStr_SPI2B);

    GPIO_InitTypeDef initStr_LED = {GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9,
                                GPIO_MODE_OUTPUT_PP,
                                GPIO_NOPULL,
                                GPIO_SPEED_FREQ_LOW,
                                0};
    HAL_GPIO_Init(GPIOC, &initStr_LED);

    // MOSI=PC3 [AF1]
    GPIO_InitTypeDef initStr_SPI2C = {GPIO_PIN_3,
                                      GPIO_MODE_AF_PP,
                                      GPIO_NOPULL,
                                      GPIO_SPEED_FREQ_HIGH,
                                      GPIO_AF1_SPI2};
    HAL_GPIO_Init(GPIOC, &initStr_SPI2C);

    // TX: CSN=PB12, CE=PB1
    // RX: CSN=PB10, CE=PB11
    GPIO_InitTypeDef initStr_NRF = {GPIO_PIN_1 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12,
                                    GPIO_MODE_OUTPUT_PP,
                                    GPIO_NOPULL,
                                    GPIO_SPEED_FREQ_LOW,
                                    0};
    HAL_GPIO_Init(GPIOB, &initStr_NRF);

    hspi2.Instance               = SPI2;
    hspi2.Init.Mode              = SPI_MODE_MASTER;
    hspi2.Init.Direction         = SPI_DIRECTION_2LINES;
    hspi2.Init.DataSize          = SPI_DATASIZE_8BIT;
    hspi2.Init.CLKPolarity       = SPI_POLARITY_LOW;
    hspi2.Init.CLKPhase          = SPI_PHASE_1EDGE;
    hspi2.Init.NSS               = SPI_NSS_SOFT;
    hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
    hspi2.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    HAL_SPI_Init(&hspi2);
    }

void USART3_Init(void) {
    __HAL_RCC_USART3_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // PA4=TX, PA5=RX
    GPIO_InitTypeDef initStr_UART = {GPIO_PIN_4 | GPIO_PIN_5,
                                     GPIO_MODE_AF_PP,
                                     GPIO_NOPULL,
                                     GPIO_SPEED_FREQ_HIGH,
                                     GPIO_AF1_USART3};
    HAL_GPIO_Init(GPIOA, &initStr_UART);

    huart3.Instance        = USART3;
    huart3.Init.BaudRate   = 115200;
    huart3.Init.WordLength = UART_WORDLENGTH_8B;
    huart3.Init.StopBits   = UART_STOPBITS_1;
    huart3.Init.Parity     = UART_PARITY_NONE;
    huart3.Init.Mode       = UART_MODE_TX_RX;
    HAL_UART_Init(&huart3);
}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    SPI2_Init();
    USART2_Init();
    HAL_Delay(100);

    NRF24_PinConfig tx_pins = {
        .cs_port = GPIOB, .cs_pin = GPIO_PIN_12,
        .ce_port = GPIOB, .ce_pin = GPIO_PIN_1
    };

    NRF24_PinConfig rx_pins = {
        .cs_port = GPIOB, .cs_pin = GPIO_PIN_10,
        .ce_port = GPIOB, .ce_pin = GPIO_PIN_11
    };

    nrf24l01p_tx_init(&tx_pins, 2400, _1Mbps);
    HAL_Delay(10);
    nrf24l01p_rx_init(&rx_pins, 2400, _1Mbps);
    HAL_Delay(10);

    // nRF24L01+ can send and receive 1-32 bytes per payload
    uint8_t tx_data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t rx_data[8] = {0};
    nrf_active = tx_pins;
uint8_t tx_config = nrf24l01p_get_status();

while (1) {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_RESET);

    nrf_active = tx_pins;
    nrf24l01p_tx_transmit(tx_data);
    HAL_Delay(20);

    nrf_active = rx_pins;
    uint8_t status = nrf24l01p_get_status();

    if (status & 0x40) {
        nrf24l01p_rx_receive(rx_data);
        if (memcmp(tx_data, rx_data, 8) == 0) {
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_9);  // Green: Success and Matched
        } 
        
        else {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);  // Oragne: Success and But Not Matched
        }
    } 
    
    else {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);  // RED: Fail
    }

    HAL_Delay(1000);
  }
}


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* User can add their own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add their own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
}
#endif /* USE_FULL_ASSERT */
