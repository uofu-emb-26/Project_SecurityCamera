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

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  HAL_IncTick();
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
    // The only interrupt that the RF chip can trigger is RX_DR (since nrf24l01p_mask_tx_interrupts())
    // was called during initialization. Therefore, at this point, it's known that the nRF24L01+'s RX FIFO
    // has valid data, so clear the interrupt flag, then start a DMA transfer to read the data.

    // Get the current status of the nRF24L01+
    uint8_t status = nrf24l01p_get_status();

    // The interrupt was for RX_DR
    if (status & 0x40) {
      // Clear the RX_DR interrupt flag
      status |= 0x40;
      write_register(NRF24L01P_REG_STATUS, status);

      // Start a DMA transfer to get the received data
      // NOTE: assumes this nRF24L01+ chip is linked to SPI1
      nrf24l01p_rx_receive_dma(&hspi1);
    }
  }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        capture_request = 1;
    }
}