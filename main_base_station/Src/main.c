// NOTE: LED meanings
//         - Red (1 second flash): unable to decompress JPEG image
//         - Orange (1 second flash): image reception error (possibly due to RF interference)

// NOTE: Interrupt priorities
//         - 0: SysTick
//         - 1: RF and Display DMA interrupts
//         - 3: External RF interrupts (EXTI-based)

#include "main.h"
#include "stm32f0xx_hal.h"
#include "jpeg_decode.h"
#include "nrf24l01p.h"
#include "nrf24l01p_ext.h"
#include <string.h>
#include <stdio.h>

// Display
SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_tx;

// RF Chip
SPI_HandleTypeDef hspi2;
DMA_HandleTypeDef hdma_spi2_rx;
DMA_HandleTypeDef hdma_spi2_tx;

// Image packets (for RF transmission)
#define DATA_PER_PACKET (NRF24L01P_PAYLOAD_LENGTH - 4) // 2 bytes (packet_id) + 2 bytes (total_packets)
typedef struct {
    uint16_t packet_id;
    uint16_t total_packets;
    uint8_t  data[DATA_PER_PACKET];
} __attribute__((packed)) ImagePacket;

#define MAX_JPEG_SIZE 10000 // Max supported JPEG size in bytes
#define MAX_PACKETS ((MAX_JPEG_SIZE + DATA_PER_PACKET - 1) / DATA_PER_PACKET)

// RF packets are loaded into this buffer before running jpeg_decode_run() on it
// This takes the majority of this STM32's RAM
uint8_t image_buffer[MAX_JPEG_SIZE] = {0};

// Determines whether each packet for a transmission has been received
// 0 means the packet hasn't been received yet
// 1 means the packet has been received
// Values are stored as a bitfield, so each packet id must be checked by dividing by 32
// to index into this array and then masking to find the corresponding bit's value
uint32_t packet_received[(MAX_PACKETS + 31) / 32] = {0};
#define BITFIELD32_IS_BIT_SET(bitfield, bit) (!!((bitfield)[(bit) / 32] & (1U << ((bit) % 32))))
#define BITFIELD32_SET_BIT(bitfield, bit) ((bitfield)[(bit) / 32] |= (1U << ((bit) % 32)))
#define BITFIELD32_CLEAR_BIT(bitfield, bit) ((bitfield)[(bit) / 32] &= ~(1U << ((bit) % 32)))

// The nRF24L01+ transmits a maximum of ~41 bytes per packet for a 32 byte packet.
// At the 2 Mbps transmission rate, this means that ~6098 packets will be transmitted per second,
// which is ~0.164 ms per packet. If, in the middle of a transaction, a packet hasn't been received
// for the equivalent of 100 packets (16.4 ms) + some fudge factor, stop trying to receive this image
// and reset for the next image.
#define MS_UNTIL_RESET 20

int main(void)
{
    // Core initialization
    HAL_Init();
    SystemClock_Config();

    // Hardware initialization
    GPIO_Init();
    EXTI_Init();
    DMA_Init();

    // Communication initialization
    SPI1_Init();
    SPI2_Init();

    // Screen initialization
    ili9341_init(
        &hspi1,
        TFT_RST_GPIO_Port, TFT_RST_Pin,
        TFT_CS_GPIO_Port,  TFT_CS_Pin,
        TFT_DC_GPIO_Port,  TFT_DC_Pin,
        isoLandscape,
        NULL, 0,
        NULL, 0,
        0, -1
    );

    // RF Chip initialization
    NRF24_PinConfig rx_pins = {
        .cs_port = GPIOB, .cs_pin = GPIO_PIN_12,
        .ce_port = GPIOB, .ce_pin = GPIO_PIN_11
    };
    nrf24l01p_rx_init(&rx_pins, 2400, _1Mbps); // TODO: change to _2Mbps
    nrf24l01p_mask_tx_interrupts();

    uint16_t total_packets = 0;
    uint16_t packets_remaining = 0;
    uint32_t last_packet_time = 0;
    uint32_t flash_red = 0;
    uint32_t flash_orange = 0;
    ImagePacket *pkt = (ImagePacket *)nrf_rx_buf.rx_data;
    while (1)
    {
        if (nrf_data_received)
        {
            nrf_data_received = 0;

            if (pkt->total_packets <= MAX_PACKETS) // Ignore images that are too large
            {
                // Initialization for the first packet of a new image
                if (total_packets == 0)
                {
                    total_packets = pkt->total_packets;
                    packets_remaining = pkt->total_packets;
                }

                // Only process the packet if it's valid (belongs to the current image and has a valid packet ID) and new
                if ((pkt->total_packets == total_packets) && (pkt->packet_id < total_packets) && !(BITFIELD32_IS_BIT_SET(packet_received, pkt->packet_id)))
                {
                    // As a 32-bit value, this won't overflow for ~50 days.
                    // Only valid packets reset the last packet time
                    last_packet_time = HAL_GetTick();

                    uint32_t offset = pkt->packet_id * DATA_PER_PACKET;
                
                    // Move the data portion of the packet into the JPEG image buffer
                    memcpy(&image_buffer[offset], pkt->data, DATA_PER_PACKET);

                    // Mark this packet as received
                    BITFIELD32_SET_BIT(packet_received, pkt->packet_id);
                    packets_remaining--;

                    // All packets for this image have been received, so pass the image off to the
                    // JPEG decompression algorithm to render it on the screen.
                    if (!packets_remaining)
                    {
                        // The image buffer may have some padding. The TJpegDec library supports this: if the 
                        // passed image length is greater than the actual image size, the TJpegDec library stops
                        // parsing at the end-of-JPEG marker.
                        uint32_t total_len = total_packets * DATA_PER_PACKET;
                        
                        // Render the JPEG image on screen
                        if (!jpeg_decode_run(image_buffer, total_len))
                        {
                            // Flash the red LED if the JPEG couldn't be decompressed and drawn
                            flash_red = HAL_GetTick() | 1; // Make this isn't 0 so that it triggers (negligible effect for 1000ms flash)
                        }

                        // Reset packet status
                        total_packets = 0;
                        memset(packet_received, 0, sizeof(packet_received));
                    }
                }
            }
        }

        // If we've stopped receiving packets in the middle of an image (possibly due to RF interference),
        // just drop this image and reset for the next one.
        if ((total_packets > 0) && ((HAL_GetTick() - last_packet_time) > MS_UNTIL_RESET))
        {
            total_packets = 0;
            memset(packet_received, 0, sizeof(packet_received));

            // Flash the orange LED
            flash_orange = HAL_GetTick() | 1;
        }

        // LED flash logic
        led_flash_handler(&flash_red, 1000, RED_PIN);
        led_flash_handler(&flash_orange, 1000, ORANGE_PIN);
    }
}

void led_flash_handler(uint32_t *flash_var, uint16_t time_ms, uint16_t pin)
{
    // Use flash_var as the indicator for whether the LED should flash as well as the amount of time
    // it should flash for
    if (*flash_var)
        {
            // Turn the LED on if less than the requested amount of time has passed
            if ((HAL_GetTick() - *flash_var) < time_ms)
            {
                HAL_GPIO_WritePin(GPIOC, pin, GPIO_PIN_SET);
            }
            // Otherwise, turn it back off
            else
            {
                HAL_GPIO_WritePin(GPIOC, pin, GPIO_PIN_RESET);
                // Mark this flash event as handled
                *flash_var = 0;
            }
        }
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

void SPI1_Init(void)
{
    hspi1.Instance               = SPI1;
    hspi1.Init.Mode              = SPI_MODE_MASTER;
    hspi1.Init.Direction         = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize          = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity       = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase          = SPI_PHASE_1EDGE;
    hspi1.Init.NSS               = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
    hspi1.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode            = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial     = 7;
    hspi1.Init.CRCLength         = SPI_CRC_LENGTH_DATASIZE;
    if (HAL_SPI_Init(&hspi1) != HAL_OK) Error_Handler();
}

void SPI2_Init(void)
{
    hspi2.Instance               = SPI2;
    hspi2.Init.Mode              = SPI_MODE_MASTER;
    hspi2.Init.Direction         = SPI_DIRECTION_2LINES;
    hspi2.Init.DataSize          = SPI_DATASIZE_8BIT;
    hspi2.Init.CLKPolarity       = SPI_POLARITY_LOW;
    hspi2.Init.CLKPhase          = SPI_PHASE_1EDGE;
    hspi2.Init.NSS               = SPI_NSS_SOFT;
    hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
    hspi2.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    if (HAL_SPI_Init(&hspi2) != HAL_OK) Error_Handler();
}

void GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    // Initial values for screen control pins
    HAL_GPIO_WritePin(GPIOA, TFT_CS_Pin|TFT_DC_Pin|TFT_RST_Pin, GPIO_PIN_SET);

    // Screen control pin initialization
    GPIO_InitStruct.Pin   = TFT_DC_Pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(TFT_DC_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin   = TFT_CS_Pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(TFT_CS_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin   = TFT_RST_Pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(TFT_RST_GPIO_Port, &GPIO_InitStruct);

    // RF chip pin initialization (CSN=PB12 and CE=PB11)
    GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_11;
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

    // Initialize and drive PC0 high to prevent conflicts with gyroscope on SPI2
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);
}

void EXTI_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    // Configure PB2 as an External Interrupt (Falling Edge)
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    // The nRF24 IRQ pin is active low, so pull it up
    GPIO_InitStruct.Pull = GPIO_PULLUP; 
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Enable the NVIC interrupt for EXTI lines 2 and 3
    // RF chip interrupts require blocking SPI transactions, so they have the lowest priority
    HAL_NVIC_SetPriority(EXTI2_3_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);
}

void DMA_Init(void)
{
  // Enable DMA1 clock
  __HAL_RCC_DMA1_CLK_ENABLE();

  // RF DMA Interrupt
  HAL_NVIC_SetPriority(DMA1_Channel4_5_6_7_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_5_6_7_IRQn);

  // Display DMA Interrupt
  HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);
}

void Error_Handler(void)
{
    __disable_irq();
    while (1) {}
}
