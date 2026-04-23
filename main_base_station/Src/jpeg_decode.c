#include "jpeg_decode.h"
#include "tjpgd.h"
#include "stm32f0xx_hal.h"
#include "images.h"

static const uint8_t *s_jpeg_buf;
static uint32_t s_jpeg_len;
static uint32_t s_pos;

// The work area used by the TJpegDec library
// It should be word-aligned
uint8_t jpeg_workarea[3100] __attribute__((aligned(4)));

void uart3_write_string(const char *s);

static size_t input_func(JDEC *jdec, uint8_t *buf, size_t ndata)
{
    uint32_t remain = s_jpeg_len - s_pos;
    if (ndata > remain) ndata = remain;
    if (buf) memcpy(buf, s_jpeg_buf + s_pos, ndata);
    s_pos += ndata;
    return ndata;
}

static int output_func(JDEC *jdec, void *bitmap, JRECT *rect)
{
    // Calculate the screen region that should be updated
    uint16_t *pix = (uint16_t *)bitmap;
    uint16_t w = rect->right - rect->left + 1;
    uint16_t h = rect->bottom - rect->top + 1;
    uint32_t count = (uint32_t)w * h;

    // Tell the screen where the following data should be drawn
    ili9341_spi_tft_set_address_rect(&lcd_global, rect->left, rect->top, rect->right, rect->bottom);

    // Start the transaction and indicate that this is data, not a command
    ili9341_spi_tft_select(&lcd_global);
    HAL_GPIO_WritePin(lcd_global.data_command_port, lcd_global.data_command_pin, GPIO_PIN_SET);

    // Swap data region to the big-endian format required by the screen
    ili9341_array_endian_swap(pix, count);

    // Send the data and end the transaction
    HAL_SPI_Transmit(lcd_global.spi_hal, (uint8_t *)pix, count * 2, HAL_MAX_DELAY);
    ili9341_spi_tft_release(&lcd_global);

    return 1;
}

uint8_t jpeg_decode_run(const uint8_t *jpeg, uint32_t len)
{
    JDEC    jdec;

    s_jpeg_buf = jpeg;
    s_jpeg_len = len;
    s_pos      = 0;

    // Set up for decompression algorithm
    if (jd_prepare(&jdec, input_func, jpeg_workarea, sizeof(jpeg_workarea), NULL) != JDR_OK) 
    {
        return 0;
    }

    // Perform decompression
    if (jd_decomp(&jdec, output_func, 0) != JDR_OK)
    {
        return 0;
    }

    return 1;
}