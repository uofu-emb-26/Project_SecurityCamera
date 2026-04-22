#include "main.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx_it.h"
#include "nrf24l01p.h"
#include "nrf24l01p_ext.h"
#include "stm32f0xx_hal_tim.h"

// RF chip
extern SPI_HandleTypeDef hspi1;
extern DMA_HandleTypeDef hdma_spi1_rx;
extern DMA_HandleTypeDef hdma_spi1_tx;

// Camera interrupt timer
extern SPI_HandleTypeDef hspi2;
extern I2C_HandleTypeDef hi2c1;
extern TIM_HandleTypeDef htim2;
extern volatile uint8_t capture_request;

// Camera/RF transmission
extern volatile uint8_t rf_tx_ready;
extern volatile uint8_t rf_tx_error;
// Whether a frame is actively being transmitted
extern volatile uint8_t transmitting_frame;

volatile uint32_t flash_red = 0;
volatile uint32_t flash_orange = 0;
volatile uint32_t flash_blue = 0;
volatile uint32_t flash_green = 0;

/******************************************************************************/
/*           Cortex-M0 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
   while (1)
  {
  }
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  while (1)
  {
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
}

void led_flash_handler(volatile uint32_t *flash_var, uint16_t time_ms, uint16_t pin)
{
    // Use flash_var as the indicator for whether the LED should flash as well as the amount of time
    // it should flash for
    if (*flash_var == 1)
    {
        // Make this isn't below 3 for code below (negligible effect for high flash values)
        *flash_var = HAL_GetTick() | 3;
    }
    
    if (*flash_var > 1)
    {
        // Turn the LED on if less than the requested amount of time has passed
        if ((HAL_GetTick() - *flash_var) < time_ms)
        {
            HAL_GPIO_WritePin(GPIOC, pin, GPIO_PIN_SET);
        }
        // Otherwise, turn it back off
        else
        {
            HAL_GPIO_WritePin(GPIOC, pin, GPIO_PIN_RESET);
            // Mark this flash event as handled
            *flash_var = 0;
        }
    }
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  HAL_IncTick();

  // LED flash logic
  led_flash_handler(&flash_blue, 100, BLUE_PIN);
  led_flash_handler(&flash_red, 100, RED_PIN);
  led_flash_handler(&flash_orange, 100, ORANGE_PIN);
  led_flash_handler(&flash_green, 100, GREEN_PIN);
}

/******************************************************************************/
/* STM32F0xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f0xx.s).                    */
/******************************************************************************/

void TIM2_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim2);
}

void DMA1_Channel2_3_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_spi1_rx);
  HAL_DMA_IRQHandler(&hdma_spi1_tx);
}

void EXTI2_3_IRQHandler(void)
{
  // Trigger the generic HAL callback
  HAL_GPIO_EXTI_IRQHandler(RF_INT_PIN);
}

// Fires when HAL_SPI_TransmitReceive_DMA completes
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
  // RF Chip SPI interface
  if(hspi->Instance == SPI1) {
    cs_high(); // End the SPI transaction
    nrf_data_buf_lock = 0; // Release the lock on the data buffers
    nrf_data_received = 1; // Indicate that data has been received

    // NOTE: to detect when the FIFO has more, unread data, check the FIFO status register
    // here and then set nrf_more_data = 1 if another packet is present.
  }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == RF_INT_PIN) {
    // On the transmit side, the RF chip may raise either MAX_RT or TX_DS interrupts

    // Get the current status of the nRF24L01+
    uint8_t status = nrf24l01p_get_status();
    uint8_t new_status = 0;

    // If TX_DS flag set
    if (status & (1 << 5))
    {
      // Clear the status flag (by writing a 1)
      new_status |= (1 << 5);
    }

    // If MAX_RT flag set
    if (status & (1 << 4))
    {
      // Clear the status flag (by writing a 1)
      new_status |= (1 << 4);

      // Signal that an error has occurred
      rf_tx_error = 1;
    }

    // Commit the status register changes
    write_register(NRF24L01P_REG_STATUS, new_status);

    // Signal that the RF chip is ready for more data
    rf_tx_ready = 1;
  }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        // Ensure at least 4 seconds pass between capture requests
        if (!transmitting_frame)
        {
          capture_request = 1;
        }
    }
}