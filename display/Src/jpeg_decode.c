#include "jpeg_decode.h"
#include "tjpgd.h"
#include "test_image.h"

static uint32_t s_pos;

static size_t input_func(JDEC *jdec, uint8_t *buf, size_t ndata)
{
    uint32_t remain = TEST_JPEG_SIZE - s_pos;
    if (ndata > remain) ndata = remain;
    if (buf) memcpy(buf, TEST_JPEG + s_pos, ndata);
    s_pos += ndata;
    return ndata;
}

static int output_func(JDEC *jdec, void *bitmap, JRECT *rect)
{
    uint8_t col = rect->left / 16;
    uint8_t row = rect->top  / 16;
    ili9341_draw_region(&lcd_global, col, row, (uint16_t *)bitmap, 1);
    return 1;
}

void jpeg_decode_run(void)
{
    JDEC    jdec;
    uint8_t work[3800];
    s_pos = 0;
    if (jd_prepare(&jdec, input_func, work, sizeof(work), NULL) != JDR_OK) return;
    jd_decomp(&jdec, output_func, 0);
}
