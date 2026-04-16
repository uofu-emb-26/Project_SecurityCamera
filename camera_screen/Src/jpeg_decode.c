#include "jpeg_decode.h"
#include "tjpgd.h"
// #include "test_image.h"
#include "stm32f0xx_hal.h"
#include "images.h"

static const uint8_t *s_jpeg_buf;
static uint32_t s_jpeg_len;
static uint32_t s_pos;

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
    // //uint8_t col = rect->left / 16;
    // //uint8_t row = rect->top  / 16;
    // //ili9341_array_endian_swap((uint16_t*)bitmap, 256);
    // //ili9341_draw_region(&lcd_global, col, row, (uint16_t *)bitmap, 1);
    // uart3_write_string("before draw_buffer\r\n");
    // ili9341_draw_buffer(&lcd_global, rect->left, rect->top, rect->right - rect->left +1 , rect->bottom - rect->top +1 , (uint16_t *) bitmap, 1);
    // uart3_write_string("after draw_buffer\r\n");
    
    // // TODO: this eliminates the benefit of DMA since TJpgDec can't move on to starting to process the next
    // // frame region until this DMA transfer completes. However, `bitmap` comes from `rect` in mcu_output() (tjpgd.c).
    // // This (`rect`) is a local variable, so `bitmap` ceases to be valid as soon as this function (output_func()) returns.
    // // Options:
    // //   #1: Modify the TJpgDec library so `rect` in mcu_output() is globally accessible, then insert the while loop
    // //       below in mcu_output() right before it modifies `rect`. This will allow TJpgDec to continue with
    // //       processing the next frame. Then, if the write of the last frame hasn't completed, the pipeline will halt.
    // //       This will prevent unneeded memory copies, and it's likely the best option.
    // //   #2: Copy bitmap into another memory address prior to starting the DMA transfer. This will incur overhead
    // //       for copying the memory, which isn't ideal.
    // // while(HAL_SPI_GetState(lcd_global.spi_hal) != HAL_SPI_STATE_READY)
    // // {

    // // }
    // return 1;

    uint16_t *pix = (uint16_t *)bitmap;
    uint16_t w = rect->right - rect->left + 1;
    uint16_t h = rect->bottom - rect->top + 1;
    uint32_t count = (uint32_t)w * h;

    ili9341_spi_tft_set_address_rect(&lcd_global, rect->left, rect->top, rect->right, rect->bottom);

    ili9341_spi_tft_select(&lcd_global);
    HAL_GPIO_WritePin(lcd_global.data_command_port, lcd_global.data_command_pin, GPIO_PIN_SET);
    ili9341_array_endian_swap(pix, count);
    HAL_SPI_Transmit(lcd_global.spi_hal, (uint8_t *)pix, count * 2, HAL_MAX_DELAY);

    ili9341_spi_tft_release(&lcd_global);

    return 1;
}

void jpeg_decode_run(const uint8_t *jpeg, uint32_t len)
{
    JDEC    jdec;
    static uint8_t work[3800];
    
    s_jpeg_buf = jpeg;
    s_jpeg_len = len;
    s_pos      = 0;

    if (jd_prepare(&jdec, input_func, work, sizeof(work), NULL) != JDR_OK) return;
    jd_decomp(&jdec, output_func, 0);
}
