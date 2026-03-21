#include "camera.h"

I2C_HandleTypeDef hi2c1;

int main(void)
{
    HAL_Init();
    // ... clock and I2C init ...
    
    camera_init(&hi2c1);

    while(1)
    {
        uint32_t len = camera_capture_frame();
        if (len > 0)
        {
            const uint8_t *jpeg = camera_get_buffer();
            // jpeg points to raw JPEG bytes
            // len is number of valid bytes
            // feed into TJPGD here
        }
    }
}