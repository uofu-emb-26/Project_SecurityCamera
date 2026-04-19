// TODO: pin remappings
//   - PB13 -> PB10 (Camera SCK)
//   - PB15 -> PC3 (Camera MOSI)
//   - Drive PC0 high
//   - PB1 -> PB10 (RF CE)

#include "main.h"

// Camera
SPI_HandleTypeDef hspi2;
I2C_HandleTypeDef hi2c1;
TIM_HandleTypeDef htim2;

// RF Chip
SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_rx;
DMA_HandleTypeDef hdma_spi1_tx;

// Flag raised by camera timer interrupt when it's time to capture a frame
volatile uint8_t capture_request = 0;

// Variables for transmitting JPEG data from camera over RF chip.
// Whether the RF chip is ready to transmit another frame
// (Start out ready to transmit first chunk)
volatile uint8_t rf_tx_ready = 1;
// Whether a frame is actively being transmitted
uint8_t transmitting_frame = 0;
// JPEG position information
uint32_t jpeg_index = 0;
uint32_t jpeg_len = 0;
const uint8_t *jpeg_buf = NULL;
// Raised if the RF chip fails to transmit
volatile uint8_t rf_tx_error = 0;

// Image packets (for RF transmission)
#define DATA_PER_PACKET (NRF24L01P_PAYLOAD_LENGTH - 4) // 2 bytes (packet_id) + 2 bytes (total_packets)
typedef struct {
    uint16_t packet_id;
    uint16_t total_packets;
    uint8_t  data[DATA_PER_PACKET];
} __attribute__((packed)) ImagePacket;

int main(void)
{
    // Core initialization
    HAL_Init();
    SystemClock_Config();

    // Hardware initialization
    GPIO_Init();
    EXTI_Init();
    DMA_Init();
    TIM2_Init();

    // Communication initialization
    SPI1_Init();
    SPI2_Init();
    I2C1_Init();
    uart3_Init();

    // Camera initialization
    camera_init(&hi2c1);

    // RF Chip initialization
    NRF24_PinConfig tx_pins = {
        .cs_port = GPIOB, .cs_pin = GPIO_PIN_12,
        .ce_port = GPIOB, .ce_pin = GPIO_PIN_1
    };
    nrf24l01p_tx_init(&tx_pins, 2400, _1Mbps); // TODO: change to _2Mbps

    // Only act on interrupts for transmission
    nrf24l01p_mask_rx_interrupts();

    ImagePacket *pkt = (ImagePacket *)nrf_tx_buf.tx_data;
    while (1)
    {
        // Get a new camera frame
        if (capture_request && !transmitting_frame)
        {
            // Acknowledge the timer-generated request for a new frame
            capture_request = 0;

            // Copy a compressed image out of the camera's memory into the STM's memory
            jpeg_len = camera_capture_frame();

            // If an image was captured, start the process of transmitting it into the RF chip's TX FIFO
            if (jpeg_len > 0)
            {
                jpeg_buf = camera_get_buffer();
                jpeg_index = 0;
                transmitting_frame = 1;
            }
        }

        // Transmit 32 bytes of the current camera frame to the RF chip TX FIFO
        if (rf_tx_ready && transmitting_frame)
        {
            // TODO: check status of rf_tx_error. If this is set, it means some bytes of the image failed to transmit.
            //       If this case is hit, the synchronization with the base station must be preserved.
            //       Solution: Wait for at least 5 ms, then reset with next image

            uint32_t bytes_remaining = jpeg_len - jpeg_index;

            if (bytes_remaining > 0)
            {
                // Send either DATA_PER_PACKET bytes, or however many are left
                uint8_t chunk_size = (bytes_remaining >= DATA_PER_PACKET) ? DATA_PER_PACKET : bytes_remaining;

                // Create the packet header
                pkt->packet_id = jpeg_index / DATA_PER_PACKET;
                pkt->total_packets = (jpeg_len + DATA_PER_PACKET - 1) / DATA_PER_PACKET;

                // Copy the next JPEG bytes into the data section of the transmission packet
                memcpy(pkt->data, &jpeg_buf[jpeg_index], chunk_size);

                // Zero-out the remaining memory for the final packet
                if (chunk_size < DATA_PER_PACKET) 
                {
                    uint8_t padding_bytes = DATA_PER_PACKET - chunk_size;
                    memset(&pkt->data[chunk_size], 0, padding_bytes);
                }

                // Start copying this chunk into the RF chip's FIFO
                rf_tx_ready = 0;
                nrf24l01p_tx_transmit_dma(&hspi1);

                jpeg_index += chunk_size;
            }
            // Every chunk for this JPEG image has been transmitted
            else
            {
                transmitting_frame = 0;
            }
        }
    }
}

void uart3_write_char(char c)
{
    while ((USART3->ISR & USART_ISR_TXE) == 0) {}
    USART3->TDR = (uint8_t)c;
}

void uart3_write_string(const char *s)
{
    while (*s) uart3_write_char(*s++);
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48;
    RCC_OscInitStruct.HSI48State     = RCC_HSI48_ON;
    RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) Error_Handler();
    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_HSI48;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) Error_Handler();
}

void uart3_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
    RCC->AHBENR  |= RCC_AHBENR_GPIOCEN;

    GPIOC->MODER &= ~((3U<<(10*2)) | (3U<<(11*2)));
    GPIOC->MODER |=  ((2U<<(10*2)) | (2U<<(11*2)));

    GPIOC->AFR[1] &= ~((0xF << ((10-8)*4)) | (0xF << ((11-8)*4)));
    GPIOC->AFR[1] |=  ((1   << ((10-8)*4)) | (1   << ((11-8)*4)));

    USART3->BRR = (uint16_t)(48000000 / 1500000);
    USART3->CR1 = USART_CR1_TE | USART_CR1_RE;
    USART3->CR1 |= USART_CR1_UE;
}

void TIM2_Init(void){
    __HAL_RCC_TIM2_CLK_ENABLE();

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 47999;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 3999;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    HAL_TIM_Base_Init(&htim2);

    HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);

    HAL_TIM_Base_Start_IT(&htim2);
}

void I2C1_Init(void)
{
    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x10805E89;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    HAL_I2C_Init(&hi2c1);
}

// RF Chip
void SPI1_Init(void)
{
    hspi1.Instance               = SPI1;
    hspi1.Init.Mode              = SPI_MODE_MASTER;
    hspi1.Init.Direction         = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize          = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity       = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase          = SPI_PHASE_1EDGE;
    hspi1.Init.NSS               = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
    hspi1.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    if (HAL_SPI_Init(&hspi1) != HAL_OK) Error_Handler();
}

// Camera
void SPI2_Init(void)
{
    hspi2.Instance = SPI2;
    hspi2.Init.Mode = SPI_MODE_MASTER;
    hspi2.Init.Direction = SPI_DIRECTION_2LINES;
    hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi2.Init.NSS = SPI_NSS_SOFT;
    hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
    hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
    HAL_SPI_Init(&hspi2);
}

void GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    // Camera CS pin initialization
    GPIO_InitStruct.Pin = OUT_CS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(OUT_CS_PORT, &GPIO_InitStruct);

    // Camera CS Pin starts high
    OUT_CS_HIGH();

    // RF chip pin initialization (CSN=PB12 and CE=PB1)
    GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // LED initialization
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void EXTI_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    // Configure PB2 as an External Interrupt (Falling Edge) for the RF chip
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    // The nRF24 IRQ pin is active low, so pull it up
    GPIO_InitStruct.Pull = GPIO_PULLUP; 
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Enable the NVIC interrupt for EXTI lines 2 and 3
    HAL_NVIC_SetPriority(EXTI2_3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);
}

void DMA_Init(void)
{
  // Enable DMA1 clock
  __HAL_RCC_DMA1_CLK_ENABLE();

  // RF DMA Interrupt
  HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);
}

void Error_Handler(void)
{
    __disable_irq();
    while (1) {}
}
