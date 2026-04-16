/*
 * Created by Zachary Ward to extend the nRF24L01+ library with DMA capability
 * 
 * Use:
 *   - Call either nrf24l01p_tx_init() or nrf24l01p_rx_init(), and then call nrf24l01p_mask_tx_interrupts()
 *     to setup the interrupts for use with this library extension
 *   - To transmit data, write into nrf_tx_buf.tx_data, then call nrf24l01p_tx_transmit_dma()
 *   - To receive data, read from nrf_rx_buf when nrf_data_received is 1
 *   - Data is available in nrf_rx_buf when nrf_data_received is 1 (either a status or 
 *     a status and payload depending on whether the transmit or the receive function
 *     was called last).
 *   - Don't call any read or write functions from the original library when
 *     nrf_data_buf_lock is 1 (this means a transfer is ongoing in the background)
 */

#ifndef __NRF24L01P_EXT_H__
#define __NRF24L01P_EXT_H__

#include "nrf24l01p.h"
#include "stm32f0xx_hal.h"
#include <stdint.h>
#include <string.h>

#define RF_INT_PIN GPIO_PIN_2

extern SPI_HandleTypeDef hspi2;
extern DMA_HandleTypeDef hdma_spi2_rx;
extern DMA_HandleTypeDef hdma_spi2_tx;

// Data type to allow either the raw data buffer or the status + data
// to be accessed independently
typedef union
{
    uint8_t raw_data[NRF24L01P_PAYLOAD_LENGTH + 1];

    struct {
        uint8_t status;
        uint8_t rx_data[NRF24L01P_PAYLOAD_LENGTH];
    };
} NRF24_RxBuffer;

// Data type to allow either the raw data buffer or the command + data
// to be accessed independently
typedef union
{
    uint8_t raw_data[NRF24L01P_PAYLOAD_LENGTH + 1];

    struct {
        uint8_t command;
        uint8_t tx_data[NRF24L01P_PAYLOAD_LENGTH];
    };
} NRF24_TxBuffer;

// Data buffer for receiving and transmitting data
extern volatile NRF24_RxBuffer nrf_rx_buf;
extern volatile NRF24_TxBuffer nrf_tx_buf;
// Synchronization lock for both data buffers
extern volatile uint8_t nrf_data_buf_lock;
// Flag to indicate that data has been received
extern volatile uint8_t nrf_data_received;

// Flag to indicate when the nRF24L01+ FIFO has more data (currently unused)
extern volatile uint8_t nrf_more_data;

int nrf24l01p_rx_receive_dma(SPI_HandleTypeDef *hspi);
int nrf24l01p_tx_transmit_dma(SPI_HandleTypeDef *hspi);

int nrf24l01p_read_rx_fifo_dma(SPI_HandleTypeDef *hspi);
int nrf24l01p_write_tx_fifo_dma(SPI_HandleTypeDef *hspi);

void nrf24l01p_mask_tx_interrupts(void);

#endif  // __NRF24L01P_EXT_H__