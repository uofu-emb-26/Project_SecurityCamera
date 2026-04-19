#include "camera.h"
#include "ArduCAM.h"
#include "spi.h"
#include "sccb_bus.h"
#include "main.h"

#define OV2640_I2C_ADDR 0x60
#define CAMERA_BUFFER_SIZE 10000

static I2C_HandleTypeDef *camera_i2c = 0;
static uint8_t jpeg_buffer[CAMERA_BUFFER_SIZE];
static uint32_t jpeg_length = 0;

void camera_init(I2C_HandleTypeDef *hi2c)
{
    camera_i2c = hi2c;
    sccb_bus_init(hi2c);
    sensor_addr = OV2640_I2C_ADDR;
    ArduCAM_CS_init();
    ArduCAM_Init(OV2640);
    HAL_Delay(100);
}

uint32_t camera_capture_frame(void)
{
    jpeg_length = 0;

    flush_fifo();
    clear_fifo_flag();
    start_capture();

    uint32_t timeout = HAL_GetTick();
    while (!(get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK)))
    {
        if (HAL_GetTick() - timeout > 3000)
        {
            uart3_write_string("capture timeout\r\n");
            return 0;
        }
    }

    uint32_t len = read_fifo_length();

    if (len == 0 || len >= CAMERA_BUFFER_SIZE)
    {
        uart3_write_string("bad fifo len\r\n");
        return 0;
    }

    CS_LOW();
    set_fifo_burst();

    for (uint32_t i = 0; i < len; i++)
        jpeg_buffer[i] = SPI2_ReadWriteByte(0x00);

    CS_HIGH();

    uint32_t end_index = 0;
    for (uint32_t i = 1; i < len; i++)
    {
        if (jpeg_buffer[i - 1] == 0xFF && jpeg_buffer[i] == 0xD9)
        {
            end_index = i;
            break;
        }
    }

    if (end_index == 0)
    {
        uart3_write_string("no jpeg end\r\n");
        return 0;
    }

    jpeg_length = end_index + 1;

    return jpeg_length;
}

const uint8_t* camera_get_buffer(void)
{
    return jpeg_buffer;
}

uint32_t camera_get_jpeg_length(void)
{
    return jpeg_length;
}