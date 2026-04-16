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

/**
 * @brief Transmits a buffer of pixel data to a specific region on the ILI9341 display using DMA.
 * @note Inspired by the ili9341_fill_rect() library function
 * @param lcd Pointer to the ILI9341 display structure.
 * @param x The starting X coordinate (top-left corner) of the drawing area.
 * @param y The starting Y coordinate (top-left corner) of the drawing area.
 * @param w The width of the drawing area in pixels.
 * @param h The height of the drawing area in pixels.
 * @param buffer Pointer to the array of 16-bit pixel data to be drawn.
 * @param endian_swap Flag indicating whether to perform an in-place endian swap (1) or not (0) 
 * prior to the DMA transfer.
 */
void ili9341_draw_buffer(ili9341_t *lcd, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *buffer, uint8_t endian_swap)
{
    // Error checking
    if (lcd == NULL || buffer == NULL)
    {
        return;
    }

    // Swap the endianness if needed
    if (endian_swap)
    {
        ili9341_array_endian_swap(buffer, w*h);
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
    HAL_SPI_Transmit_DMA(lcd->spi_hal, (uint8_t *)buffer, total_bytes);
}

/**
 * @brief Draws a 16x16 pixel region at a grid coordinate on the ILI9341 display using DMA.
 * @param lcd Pointer to the ILI9341 display structure.
 * @param column The grid column index (0 to 19, where 0 is the left edge).
 * @param row The grid row index (0 to 14, where 0 is the top edge).
 * @param buffer Pointer to the array of 256 (16x16) 16-bit pixels to be drawn.
 * @param endian_swap Flag indicating whether to perform an in-place endian swap (1) or not (0) 
 * prior to the DMA transfer.
 */
void ili9341_draw_region(ili9341_t *lcd, uint8_t column, uint8_t row, uint16_t *buffer, uint8_t endian_swap)
{
    // A 320x240 pixel screen has 15 rows and 20 columns of 16x16 pixel regions
    if ((column >= 20) || (row >= 15))
    {
        return;
    }

    ili9341_draw_buffer(lcd, column * 16, row * 16, 16, 16, buffer, endian_swap);
}