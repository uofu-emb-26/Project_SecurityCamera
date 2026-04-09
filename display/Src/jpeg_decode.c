#include "jpeg_decode.h"
#include "tjpgd.h"
#include "test_image.h"
#include "stm32f0xx_hal.h"
// #include "images.h"

static uint32_t s_pos;
extern unsigned char camera_jpg[];
extern unsigned int camera_jpg_size;

static size_t input_func(JDEC *jdec, uint8_t *buf, size_t ndata)
{
    uint32_t remain = camera_jpg_size - s_pos;
    if (ndata > remain) ndata = remain;
    if (buf) memcpy(buf, camera_jpg + s_pos, ndata);
    s_pos += ndata;
    return ndata;
}

static int output_func(JDEC *jdec, void *bitmap, JRECT *rect)
{
    //uint8_t col = rect->left / 16;
    //uint8_t row = rect->top  / 16;
    //ili9341_array_endian_swap((uint16_t*)bitmap, 256);
    //ili9341_draw_region(&lcd_global, col, row, (uint16_t *)bitmap, 1);
    ili9341_draw_buffer(&lcd_global, rect->left, rect->top, rect->right - rect->left +1 , rect->bottom - rect->top +1 , (uint16_t *) bitmap, 1);
    
    // TODO: this eliminates the benefit of DMA since TJpgDec can't move on to starting to process the next
    // frame region until this DMA transfer completes. However, `bitmap` comes from `rect` in mcu_output() (tjpgd.c).
    // This (`rect`) is a local variable, so `bitmap` ceases to be valid as soon as this function (output_func()) returns.
    // Options:
    //   #1: Modify the TJpgDec library so `rect` in mcu_output() is globally accessible, then insert the while loop
    //       below in mcu_output() right before it modifies `rect`. This will allow TJpgDec to continue with
    //       processing the next frame. Then, if the write of the last frame hasn't completed, the pipeline will halt.
    //       This will prevent unneeded memory copies, and it's likely the best option.
    //   #2: Copy bitmap into another memory address prior to starting the DMA transfer. This will incur overhead
    //       for copying the memory, which isn't ideal.
    while(HAL_SPI_GetState(lcd_global.spi_hal) != HAL_SPI_STATE_READY)
    {

    }
    return 1;
}

void jpeg_decode_run(void)
{
    JDEC    jdec;
    uint8_t work[5000];
    s_pos = 0;
    if (jd_prepare(&jdec, input_func, work, sizeof(work), NULL) != JDR_OK) return;
    jd_decomp(&jdec, output_func, 0);
}
