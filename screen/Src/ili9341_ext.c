// Created by Zachary Ward to extend the ILI9341 TFT driver to write raw buffer
// data using DMA.

#include "ili9341_ext.h"

// The system starts with no DMA transfer in progress
volatile uint8_t display_dma_busy = 0;

// Handler for Display SPI interface (runs when a DMA transfer completes)
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    // Display uses SPI1
    if (hspi->Instance == SPI1)
    {
        // End the SPI transaction by putting the TFT CS pin back high
        ili9341_spi_tft_release(&lcd_global);

        // Signal that the DMA transaction is complete
        display_dma_busy = 0;
    }
}

// Inspired by ili9341_fill_rect()
void ili9341_draw_buffer(ili9341_t *lcd, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *buffer)
{
    // Error checking
    if (lcd == NULL || buffer == NULL)
    {
        return;
    }

    // If another DMA transfer has been commanded while an existing transfer
    // is in progress, wait until the existing transfer completes before
    // starting the new one.
    while (display_dma_busy) {}

    // Signal that a DMA transaction is in progress
    display_dma_busy = 1;

    // Tell the display which region will be written
    ili9341_spi_tft_set_address_rect(lcd, x, y, (x + w - 1), (y + h - 1));
    // Pull the display's CS pin low (tells the display it's being addressed for the SPI protocol)
    ili9341_spi_tft_select(lcd);

    // Indicate this transmission is data, not a command
    HAL_GPIO_WritePin(lcd->data_command_port, lcd->data_command_pin, __GPIO_PIN_SET__);

    uint32_t total_bytes = w * h * BYTES_PER_PIXEL;
    // May require an endianness swap
    HAL_SPI_Transmit_DMA(lcd->spi_hal, (uint8_t *)buffer, total_bytes);
}