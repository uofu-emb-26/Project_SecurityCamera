#include "camera.h"

#define OV2640_I2C_ADDR 0x60

static uint8_t frame_buffer[FRAME_BUFFER_SIZE];
static uint32_t frame_size = 0;

void camera_init(I2C_HandleTypeDef *hi2c)
{
    sccb_bus_init(hi2c);
    sensor_addr = OV2640_I2C_ADDR;
    ArduCAM_CS_init();
    ArduCAM_Init(OV2640);
    HAL_Delay(100);
}

uint32_t camera_capture_frame(void)
{
    frame_size = 0;

    flush_fifo();
    clear_fifo_flag();
    start_capture();

    uint32_t timeout = HAL_GetTick();
    while (!(get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK)))
    {
        if (HAL_GetTick() - timeout > 3000)
            return 0;
    }

    uint32_t len = read_fifo_length();
    if (len == 0 || len >= FRAME_BUFFER_SIZE)
        return 0;

    CS_LOW();
    set_fifo_burst();

    for (uint32_t i = 0; i < len; i++)
        frame_buffer[i] = SPI1_ReadWriteByte(0x00);

    CS_HIGH();

    frame_size = len;
    return frame_size;
}

const uint8_t* camera_get_buffer(void)
{
    return frame_buffer;
}

uint32_t camera_get_frame_size(void)
{
    return frame_size;
}