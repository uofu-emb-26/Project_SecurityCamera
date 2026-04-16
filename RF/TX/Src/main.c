#include "stm32f0xx_hal.h"
#include "nrf24l01p.h"
#include "nrf24l01p_ext.h"
#include "image_data.h"


#define DATA_PER_PACKET 28
#define IMG_SIZE        4096
#define TOTAL_PACKETS   ((IMG_SIZE + DATA_PER_PACKET - 1) / DATA_PER_PACKET)

typedef struct {
    uint16_t packet_id;
    uint16_t total_packets;
    uint8_t  data[DATA_PER_PACKET];
} __attribute__((packed)) ImagePacket;

extern SPI_HandleTypeDef  hspi2;
extern DMA_HandleTypeDef  hdma_spi2_rx;
extern DMA_HandleTypeDef  hdma_spi2_tx;

extern volatile NRF24_TxBuffer nrf_tx_buf;
extern volatile uint8_t nrf_data_buf_lock;

NRF24_PinConfig tx_pins = {
    .cs_port = GPIOB, .cs_pin = GPIO_PIN_12,
    .ce_port = GPIOB, .ce_pin = GPIO_PIN_1
};

void SystemClock_Config(void);
void GPIO_Init(void);
void SPI2_Init(void);
void DMA_Init(void);
void Error_Handler(void);

void GPIO_Init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitTypeDef initStr_SPI2B = {GPIO_PIN_13 | GPIO_PIN_14,
                                      GPIO_MODE_AF_PP, GPIO_NOPULL,
                                      GPIO_SPEED_FREQ_HIGH, GPIO_AF0_SPI2};
    HAL_GPIO_Init(GPIOB, &initStr_SPI2B);

    GPIO_InitTypeDef initStr_SPI2C = {GPIO_PIN_3, GPIO_MODE_AF_PP,
                                      GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH,
                                      GPIO_AF1_SPI2};
    HAL_GPIO_Init(GPIOC, &initStr_SPI2C);

    GPIO_InitTypeDef initStr_NRF = {GPIO_PIN_12 | GPIO_PIN_1,
                                    GPIO_MODE_OUTPUT_PP, GPIO_NOPULL,
                                    GPIO_SPEED_FREQ_LOW, 0};
    HAL_GPIO_Init(GPIOB, &initStr_NRF);
}

void SPI2_Init(void)
{
    __HAL_RCC_SPI2_CLK_ENABLE();

    hspi2.Instance               = SPI2;
    hspi2.Init.Mode              = SPI_MODE_MASTER;
    hspi2.Init.Direction         = SPI_DIRECTION_2LINES;
    hspi2.Init.DataSize          = SPI_DATASIZE_8BIT;
    hspi2.Init.CLKPolarity       = SPI_POLARITY_LOW;
    hspi2.Init.CLKPhase          = SPI_PHASE_1EDGE;
    hspi2.Init.NSS               = SPI_NSS_SOFT;
    hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
    hspi2.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    HAL_SPI_Init(&hspi2);
}

void DMA_Init(void)
{
    __HAL_RCC_DMA1_CLK_ENABLE();

    hdma_spi2_rx.Instance                 = DMA1_Channel4;
    hdma_spi2_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hdma_spi2_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_spi2_rx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_spi2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_spi2_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_spi2_rx.Init.Mode                = DMA_NORMAL;
    hdma_spi2_rx.Init.Priority            = DMA_PRIORITY_HIGH;
    HAL_DMA_Init(&hdma_spi2_rx);
    __HAL_LINKDMA(&hspi2, hdmarx, hdma_spi2_rx);

    hdma_spi2_tx.Instance                 = DMA1_Channel5;
    hdma_spi2_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    hdma_spi2_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_spi2_tx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_spi2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_spi2_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_spi2_tx.Init.Mode                = DMA_NORMAL;
    hdma_spi2_tx.Init.Priority            = DMA_PRIORITY_HIGH;
    HAL_DMA_Init(&hdma_spi2_tx);
    __HAL_LINKDMA(&hspi2, hdmatx, hdma_spi2_tx);

    HAL_NVIC_SetPriority(DMA1_Channel4_5_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel4_5_IRQn);
}

void DMA1_Channel4_5_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_spi2_rx);
    HAL_DMA_IRQHandler(&hdma_spi2_tx);
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    cs_high();
    nrf_data_buf_lock = 0;
    nrf24l01p_tx_irq();
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    GPIO_Init();
    DMA_Init();
    SPI2_Init();
    HAL_Delay(100);

    nrf24l01p_tx_init(&tx_pins, 2400, _1Mbps);
    HAL_Delay(10);

    ImagePacket pkt;

    while (1) 
    {
        for(uint16_t i = 0; i < TOTAL_PACKETS; i++)
        {
            pkt.packet_id = i;
            pkt.total_packets = TOTAL_PACKETS;

            uint16_t offset = i * DATA_PER_PACKET;
            uint16_t length;

            if(offset+DATA_PER_PACKET <= IMG_SIZE)
            {
                length = DATA_PER_PACKET;
            }

            else 
            {
                length = IMG_SIZE - offset;
            }

            memcpy(pkt.data, &image[offset], length); // Copy data to packet
            memcpy((void *)nrf_tx_buf.tx_data, &pkt, sizeof(ImagePacket)); // Copy packet to TX buffer
            nrf24l01p_tx_transmit_dma(&hspi2); // Send to the RF chip
            while(nrf_data_buf_lock); // wait until complete
            HAL_Delay(10);
        }
        HAL_Delay(1000);
    }
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLMUL          = RCC_PLL_MUL6;
    RCC_OscInitStruct.PLL.PREDIV          = RCC_PREDIV_DIV1;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) Error_Handler();
    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) Error_Handler();
}

void Error_Handler(void)
{
    __disable_irq();
    while (1) {}
}