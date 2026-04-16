#include "nrf24l01p_ext.h"

// Receive and transmit data buffers for a single nRF24L01+ chip
// NOTE: nrf_rx_buf.status will be updated after any read OR write command
// using nrf24l01p_rx_receive_dma() or nrf24l01p_tx_transmit_dma().
volatile NRF24_RxBuffer nrf_rx_buf = {0};
volatile NRF24_TxBuffer nrf_tx_buf = {0};

// Data buffer starts out not locked
volatile uint8_t nrf_data_buf_lock = 0;
// Start out with no data received
volatile uint8_t nrf_data_received = 0;

volatile uint8_t nrf_more_data = 0;

int nrf24l01p_rx_receive_dma(SPI_HandleTypeDef *hspi)
{
    return nrf24l01p_read_rx_fifo_dma(hspi);
}

// Prior to calling this function, the payload 
// should have already been placed into nrf_tx_buf.tx_data
// Note: this function also updates nrf_rx_buf.status with the
// current value of the nRF24L01+'s status register.
int nrf24l01p_tx_transmit_dma(SPI_HandleTypeDef *hspi)
{
    return nrf24l01p_write_tx_fifo_dma(hspi);
}

int nrf24l01p_read_rx_fifo_dma(SPI_HandleTypeDef *hspi)
{
    // Immediately return if a lock can't be acquired for the data buffers
    if (nrf_data_buf_lock)
    {
        // Return failure
        return 0;
    }

    // Acquire the lock on the data buffers
    nrf_data_buf_lock = 1;

    // Lower the data received flag (if any wasn't read, it will now be overwritten)
    nrf_data_received = 0;

    // Pack the command byte for receiving a payload, then garbage data to run the SPI clock
    // to ensure the payload is received.
    nrf_tx_buf.command = NRF24L01P_CMD_R_RX_PAYLOAD;
    memset((void *)&nrf_tx_buf.tx_data[0], 0xFF, NRF24L01P_PAYLOAD_LENGTH);

    // Pull CS low and start the transfer
    cs_low();
    HAL_SPI_TransmitReceive_DMA(hspi, (uint8_t *)&nrf_tx_buf.raw_data[0], (uint8_t *)&nrf_rx_buf.raw_data[0], NRF24L01P_PAYLOAD_LENGTH + 1);

    // NOTE: The nRF24L01+'s RX FIFO may be storing up to 3 packets. Currently, if more than one packet is in the FIFO,
    //       the extra packets are ignored by this code. As long as we keep everything synchronized, I don't anticipate
    //       that more than one packet will accumulate, so this should be fine.
    //       
    //       If this does need to be implemented, follow the implementation notes for nrf_more_data.

    // CS will be pulled back high and the buffer lock released in the transfer-complete callback

    // Return success
    return 1;
}

int nrf24l01p_write_tx_fifo_dma(SPI_HandleTypeDef *hspi)
{
    // Immediately return if a lock can't be acquired for the data buffers
    if (nrf_data_buf_lock)
    {
        // Return failure
        return 0;
    }

    // Acquire the lock on the data buffers
    nrf_data_buf_lock = 1;

    // Pack the command byte for transmitting a payload.
    // The actual payload must have already been packed into nrf_tx_buf.tx_dat
    nrf_tx_buf.command = NRF24L01P_CMD_W_TX_PAYLOAD;

    // Pull CS low and start the transfer
    cs_low();
    HAL_SPI_TransmitReceive_DMA(hspi, (uint8_t *)&nrf_tx_buf.raw_data[0], (uint8_t *)&nrf_rx_buf.raw_data[0], NRF24L01P_PAYLOAD_LENGTH + 1);

    // CS will be pulled back high and the buffer lock released in the transfer-complete callback

    // Return success
    return 1;
}

// Update the nRF24L01+'s configuration register so that only the RX_DR (data received) interrupt is enabled
void nrf24l01p_mask_tx_interrupts(void)
{
    // Read the current config register value
    uint8_t config = read_register(NRF24L01P_REG_CONFIG);

    // Set Bit 5 (MASK_TX_DS) and Bit 4 (MASK_MAX_RT) to 1 to mask them
    config |= (1 << 5) | (1 << 4);

    // Write the updated configuration back to the register
    write_register(NRF24L01P_REG_CONFIG, config);
}

// Update the nRF24L01+'s configuration register so that only the TX_DS and MAX_RT interrupts are enabled
void nrf24l01p_mask_rx_interrupts(void)
{
    // Read the current config register value
    uint8_t config = read_register(NRF24L01P_REG_CONFIG);

    // Set Bit 6 (MASK_RX_DR) to mask RX_DR interrupts
    config |= (1 << 6);

    // Clear Bit 5 (MASK_TX_DS) and Bit 4 (MASK_MAX_RT) to ensure these interrupts are enabled
    config &= ~((1 << 5) | (1 << 4));

    // Write the updated configuration back to the register
    write_register(NRF24L01P_REG_CONFIG, config);
}