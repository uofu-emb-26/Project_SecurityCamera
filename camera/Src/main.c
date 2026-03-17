#include "ArduCAM.h"
#include "spi.h"
#include "main.h"
#include "stm32f0xx_hal.h"

int main(void)
{
    HAL_Init();
    SPI1_Init();
    ArduCAM_CS_init();
    ArduCAM_Init(OV2640);
    
    while(1)
    {
        // camera loop will go here
    }
}