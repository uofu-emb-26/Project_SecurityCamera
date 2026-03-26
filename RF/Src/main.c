#include "main.h"
#include "nrf24l01p.h"

SPI_HandleTypeDef hspi2;

void SystemClock_Config(void);

void SPI2_Init(void) {
    __HAL_RCC_SPI2_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // SCK=PB13, MISO=PB14, MOSI=PB15
    GPIO_InitTypeDef initStr_SPI2 = {GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15,
                                     GPIO_MODE_AF_PP,
                                     GPIO_NOPULL,
                                     GPIO_SPEED_FREQ_HIGH,
                                     GPIO_AF0_SPI2};
    HAL_GPIO_Init(GPIOB, &initStr_SPI2);

    // CSN=PB12, CE=PB11
    GPIO_InitTypeDef initStr_NRF_GPIO = {GPIO_PIN_11 | GPIO_PIN_12,
                                         GPIO_MODE_OUTPUT_PP,
                                         GPIO_NOPULL,
                                         GPIO_SPEED_FREQ_LOW};
    HAL_GPIO_Init(GPIOB, &initStr_NRF_GPIO);

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

int main(void) {
    HAL_Init();
    
    __HAL_RCC_GPIOC_CLK_ENABLE();
    GPIO_InitTypeDef initStr_LED = {GPIO_PIN_6,
                                    GPIO_MODE_OUTPUT_PP,
                                    GPIO_NOPULL,
                                    GPIO_SPEED_FREQ_LOW};
    HAL_GPIO_Init(GPIOC, &initStr_LED);

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
    HAL_Delay(1000);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);

    SystemClock_Config();
    SPI2_Init();
    HAL_Delay(100);

    uint8_t status = nrf24l01p_get_status();

   if (status != 0xFF && status != 0x00) {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
    } else {
        while (1) {
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
            HAL_Delay(200);
        }
    }

    while (1) {}
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
