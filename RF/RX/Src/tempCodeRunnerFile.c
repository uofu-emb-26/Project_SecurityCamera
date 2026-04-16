#include "stm32f0xx_hal.h"
#include "nrf24l01p.h"
#include "nrf24l01p_ext.h"
#include <stdio.h>
#include <string.h>

#define DATA_PER_PACKET 28
#define IMG_SIZE        4096
#define TOTAL_PACKETS   ((IMG_SIZE + DATA_PER_PACKET - 1) / DATA_PER_PACKET)

typedef struct {
    uint16_t packet_id;
    uint16_t total_packets;
    uint8_t  data[28];
} __attribute__((packed)) ImagePacket;

extern SPI_HandleTypeDef  hspi2;
extern DMA_HandleTypeDef  hdma_spi2_rx;
extern DMA_HandleTypeDef  hdma_spi2_tx;
UART_HandleTypeDef huart3;

extern volatile NRF24_RxBuffer nrf_rx_buf;
extern volatile uint8_t        nrf_data_buf_lock;
extern volatile uint8_t        nrf_data_received;

uint8_t reconstructed[IMG_SIZE]       = {0};
uint8_t received_flags[TOTAL_PACKETS] = {0};

NRF24_PinConfig rx_pins = {
    .cs_port = GPIOB, .cs_pin = GPIO_PIN_10,
    .ce_port = GPIOB, .ce_pin = GPIO_PIN_11
};

void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_SPI2_Init(void);
void MX_DMA_Init(void);
void USART3_Init(void);
void Error_Handler(void);

void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
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

    GPIO_InitTypeDef initStr_NRF = {GPIO_PIN_10 | GPIO_PIN_11,
                                    GPIO_MODE_OUTPUT_PP, GPIO_NOPULL,
                                    GPIO_SPEED_FREQ_LOW, 0};
    HAL_GPIO_Init(GPIOB, &initStr_NRF);

    GPIO_InitTypeDef initStr_IRQ = {GPIO_PIN_0, GPIO_MODE_IT_FALLING,
                                    GPIO_PULLUP, GPIO_SPEED_FREQ_LOW, 0};
    HAL_GPIO_Init(GPIOA, &initStr_IRQ);

    HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);
}

void MX_SPI2_Init(void)
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

void MX_DMA_Init(void)
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

void USART3_Init(void)
{
    __HAL_RCC_USART3_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitTypeDef initStr_UART = {GPIO_PIN_4 | GPIO_PIN_5,
                                     GPIO_MODE_AF_PP, GPIO_NOPULL,
                                     GPIO_SPEED_FREQ_HIGH, GPIO_AF1_USART3};
    HAL_GPIO_Init(GPIOC, &initStr_UART);

    huart3.Instance        = USART3;
    huart3.Init.BaudRate   = 115200;
    huart3.Init.WordLength = UART_WORDLENGTH_8B;
    huart3.Init.StopBits   = UART_STOPBITS_1;
    huart3.Init.Parity     = UART_PARITY_NONE;
    huart3.Init.Mode       = UART_MODE_TX_RX;
    HAL_UART_Init(&huart3);
}

void DMA1_Channel4_5_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_spi2_rx);
    HAL_DMA_IRQHandler(&hdma_spi2_tx);
}

void EXTI0_1_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    cs_high();
    nrf_data_buf_lock = 0;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin != GPIO_PIN_0) return;

    HAL_UART_Transmit(&huart3, (uint8_t*)"irq\n", 4, 100);

    uint8_t status = nrf24l01p_get_status();
    if (status & 0x40) {  // RX_DR flag
        nrf24l01p_rx_receive_dma(&hspi2);
        while (nrf_data_buf_lock);
        nrf_data_received = 1;
    }
    nrf24l01p_clear_rx_dr();
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_SPI2_Init();
    USART3_Init();
    HAL_Delay(100);

    nrf24l01p_rx_init(&rx_pins, 2400, _1Mbps);
    HAL_Delay(10);
    nrf24l01p_flush_rx_fifo();  
    nrf24l01p_clear_rx_dr();  
    
  while (1) {
    if (nrf_data_received) {
        nrf_data_received = 0;

        ImagePacket *pkt = (ImagePacket *)nrf_rx_buf.rx_data;

        if (pkt->packet_id < TOTAL_PACKETS) {
            uint16_t offset = pkt->packet_id * DATA_PER_PACKET;
            uint16_t length;
            if (offset + DATA_PER_PACKET <= IMG_SIZE) {
                length = DATA_PER_PACKET;
            } else {
                length = IMG_SIZE - offset;
            }
            memcpy(&reconstructed[offset], pkt->data, length);
            received_flags[pkt->packet_id] = 1;
        }

        // 몇 개 받았는지 카운트해서 보내기
        uint16_t count = 0;
        for (int i = 0; i < TOTAL_PACKETS; i++) {
            if (received_flags[i]) count++;
        }
        char buf[32];
        sprintf(buf, "pkt:%d/%d\n", count, TOTAL_PACKETS);
        HAL_UART_Transmit(&huart3, (uint8_t*)buf, strlen(buf), 100);
    }
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