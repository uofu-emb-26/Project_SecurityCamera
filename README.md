# Wireless Security Camera
#### Zachary Ward, Zoey Lee, Charles Jones, Sangeun An

## Table of Contents
- [Overview](#overview)
- [Hardware](#hardware)
  - [Wiring](#wiring)
  - [Available SPI & I2C Ports](#stm32f072-discovery-spii2c-ports)
- [Software](#software)
  - [Data Flow](#data-flow)
  - [Repository Structure](#repository-structure)
- Major Components
  - [Camera](#camera)
  - [RF Communication](#rf-communication)
  - [JPEG Decompression](#jpeg-decompression)
  - [Screen](#screen)
- [Project Milestones](#milestones)

## Overview
This is the wireless security camera. It consists of two microcontroller modules: the camera and the base station. The camera captures and wirelessly transmits a video feed to the base station, which receives and displays the image feed on an integrated display. Both modules use the STM32F072 Discovery board and the nRF24L01+ RF chip. The camera module uses the ArduCAM OV2640 camera, and the base station uses a 2.8" Adafruit TFT screen with the ILI9341 chipset and a resolution of 320x240. The STM32F072 has very limited RAM, so JPEG compression is used to meet these constraints. The camera system is shown below.

// TODO: add final system picture

## Hardware
The following hardware is used to implement this project.

|    **Component**    |                  **Part**                 | **Quantity** |  **Price** |                      **Link**                      |  **Datasheets**  |
|:-------------------:|:-----------------------------------------:|:------------:|:----------:|:--------------------------------------------------:|:------------------:|
| **Microcontroller** |      STM32F072 Discovery Kit (UM1690)     |       2      |  $11.13 ea | https://estore.st.com/en/stm32f072b-disco-cpn.html | [Discovery Board](/docs/datasheets/stm32_discovery.pdf), [Processor](/docs/datasheets/stm32f072.pdf), [Core](/docs/datasheets/stm32_arm_core.pdf), [Peripherals](/docs/datasheets/stm32_peripherals.pdf) |
|      **Camera**     |       Arducam Mini 2MP Plus (OV2640)      |       1      |  $25.99 ea |               https://a.co/d/06XITQKc              | [ArduCAM](/docs/datasheets/arducam_overview.pdf), [RAM](/docs/datasheets/arducam_spi.pdf), [OV2640 Sensor](/docs/datasheets/arducam_ov2640_sensor.pdf) |
|      **Screen**     | Adafruit 2.8" 320x240 SPI ILI9341 Display |       1      |  $24.95 ea |        https://www.adafruit.com/product/1651       | [ILI9341 Chipset](/docs/datasheets/screen_ili9341_chipset.pdf), [Touchscreen](/docs/datasheets/screen_tsc2007.pdf), [Level Shifter](/docs/datasheets/screen_level_shifter.pdf), [Voltage Regulator](/docs/datasheets/screen_voltage_regulator.pdf) |
|     **RF Chip**     |                 nRF24L01+                 |       2      | $7.89 4-pk |               https://a.co/d/04dAOcaQ              | [nRF24L01+](/docs/datasheets/nrf24l01p.pdf) |

### Wiring
The wireless security camera is wired according to the diagram and tables below.

![System Wiring](/docs/img/system_connections.png "System Wiring")

#### Camera

| Camera Pin | CS | MOSI | MISO | SCK | GND | VCC | SDA | SCL |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| STM32 Pin | PA4 | PC3 | PB14 | PB10 | GND | 5V | PB7 | PB6 |

#### TX RF Chip

| NRF24L01 Pin | GND | CE | SCK | MISO | VCC | CS | MOSI | IRQ |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| STM32 Pin | GND | PC4 | PB3 | PB4 | 3V | PB12 | PB5 | PB2 |

#### RX RF Chip

| NRF24L01 Pin | IRQ | MOSI | CS | VCC | MISO | SCK | CE | GND |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| STM32 Pin | PB2 | PC3 | PB12 | 3V | PB14 | PB10 | PB11 | GND |

#### Screen

| TFT LCD Pin | GND | CS | DC | 3V3 | RST | 5V | SCK | MOSI |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| STM32 Pin | GND | PA9 | PA10 | 3V | PA8 | 5V | PB3 | PB5 |

### STM32F072 Discovery SPI/I2C Ports
The screen and RF modules both communicate over SPI. The camera has two interfaces: SPI for image data, and I2C for control. Each STM32F072 has two SPI interfaces and two I2C interfaces that each can use one of two GPIO pins for their I/O. These options, and what they conflict with on the Discovery board, are included below.

#### SPI1

| Signal | Pin | AF | Discovery Board Conflicts | Notes |
| --- | --- | --- | --- | --- |
| MISO | PB4 | AF0 | - |  |
| MISO | PA6 | AF0 | TS_G2_IO3 | **DON'T USE** without soldering SB27-32 and removing R38-40 and C26-28 |
| MOSI | PB5 | AF0 | - | |
| MOSI | PA7 | AF0 | TS_G2_IO4 | **DON'T USE** without soldering SB27-32 and removing R38-40 and C26-28 |
| SCLK | PB3 | AF0 | - | |
| SCLK | PA5 | AF0 | - | |

#### SPI2

| Signal | Pin | AF | Discovery Board Conflicts | Notes |
| --- | --- | --- | --- | --- |
| MISO | PB14 | AF0 | Gyro SDO | **AS LONG AS PC0 (Gyro CS) is driven high**, this can be used. This switches SDO to SA0 (Gyro address bit 0), meaning this pin on the Gyro is in input mode and therefore tri-stated. |
| MISO | PC2 | AF1 | Gyro Int2 | Interrupt pin driven by Gyro. **DON'T USE** (to prevent driving opposing states on the MISO line and frying something). |
| MOSI | PC3 | AF1 | - | |
| MOSI | PB15 | AF0 | Gyro SDA | **This pin should probably be avoided**. If PC0 is driven high, I2C mode is enabled on the Gyro, so this pin functions as SDA (if bits corresponding to the Gyro's address are transmitted over MOSI, the Gyro may drive the line and fry something). If PC0 is driven low, this pin functions as SDI by default (input), but this means PB14 can't be used as MISO. |
| SCLK | PB13 | AF0 | Gyro SCL | On the Gyro, this pin is only an input (either for SCL (I2C clock) or SPC (SPI clock)). **It is a good option to use for SCLK.** |
| SCLK | PB10 | AF5 | EXT/RF-E2P SCL | This only connects to a header, so it's **fine to use as the clock**. However, this has a long trace that passes all of the board's left-side pins, so it may not be great for high-speed data. |

#### I2C1

| Signal | Pin | AF | Discovery Board Conflict | Notes |
| --- | --- | --- | --- | --- |
| SCL | PB6 | AF1 | - | |
| SCL | PB8 | AF1 | - | |
| SDA | PB7 | AF1 | - | |
| SDA | PB9 | AF1 | - | |

#### I2C2

| Signal | Pin | AF | Discovery Board Conflicts | Notes |
| --- | --- | --- | --- | --- |
| SCL | PB10 | AF1 | EXT/RF-E2P SCL | Connects to a header, but otherwise isn't used |
| SCL | PB13 | AF5 | Gyro SCL | Probably used by SPI2_SCLK |
| SDA | PB11 | AF1 | EXT/RF-E2P SDA | Connects to a header, but otherwise isn't used |
| SDA | PB14 | AF5 | Gyro SDO | Used by SPI2_MISO |

## Software
### Data Flow
![System Data Flow](/docs/img/system_data_flow.png "System Data Flow")

The camera module uses a timer to read images from the ArduCAM at a consistent framerate over SPI2 into a 10KB image buffer. The RF chip supports a maximum packet size of 32 bytes, of which 4 bytes are used by the image transfer protocol to designate the total number of packets in a transaction and the current packet ID within that transaction, so the remaining 28 bytes are filled with successive chunks of the image buffer. Each constructed packet is transferred into the RF chip's TX FIFO over SPI1 using DMA to ensure the STM32's core remains available to continue interfacing with the ArduCAM.

The RF chip transmits the data packet combined with a synchronization preamble that alerts receiving RF chips of incoming data, the address of the RX chip that should receive the data, data control flags, and a CRC that allows the RX chip to verify it received the data correctly. The RX chip sends an acknowledgement when it receives data and the data's CRC is correct, so packets that are lost or corrupted in transmission aren't acknowledged and can be automatically retransmitted by the TX chip. After the RX chip validates the received data, it interrupts the base station's microcontroller, which triggers a DMA read of the packet out of the RX chip's receive FIFO over SPI2. The data received can then be unpacked and read into the correct position in the base station's image buffer using the packet ID from the image transfer protocol header.

A 320x240 pixel image with 16 bits of color resolution - the RGB565 format used by the screen - would require more than 1 MB of RAM, but the STM32F072 only has 16 KB. To remedy this, the OV2640 sensor in the ArduCAM was configured to compress the images it captures into JPEG files that fit into the microcontrollers' 10 KB image buffers. The TJpegDec library, which is optimized for JPEG decompression on low-resource microcontrollers, is then used to decompress the image one region at a time and write the result to the screen over SPI1.

To fit both the image buffer and the workspace for the TJpegDec library into the base station's 16 KB of RAM, the linker script was modified to remove heap memory, and the base station's data structures were modified to use bitmasking to pack flags into single bits.

The STM32F072 is little-endian, and the camera and RF chips both transfer data as little-endian. However, the screen is big-endian. For images to be displayed correctly, each image region decompressed by the TJpegDec library must undergo an endian swap prior to transmission to the screen. This process is accelerated with the ARM architecture's `REV16` instruction, which can perform this swap on four bytes in one clock cycle.

JPEG decompression is computationally expensive - decompressing a single 320x240 image takes approximately 5 seconds on the STM32F072 with its core clock configured to the maximum 48 MHz rate. Without a more powerful microcontroller, this means the refresh rate of the security camera's video feed is limited to ~0.2 Hz.


### Repository Structure
As this project was implemented, new directories were created as features were added. This allowed each feature to stand alone as its own project so that it could easily be rebuilt, flashed, and tested or demonstrated if needed. To this end, the directories in this repository are explained below.

#### Final Project Code
- [main_camera_module](/main_camera_module/): The final code for the Camera Module.
  - Initialize Project: `mkdir build/ && cmake -B build/ -S .` (run from the same directory as this README)
  - Build: `cmake --build build --target main_camera_module`
  - Flash: `cmake --build build --target flash_main_camera_module`
- [main_base_station](/main_base_station/): The final code for the Base Station.
  - Initialize Project: already complete (as part of main_camera_module)
  - Build: `cmake --build build --target main_base_station`
  - Flash: `cmake --build build --target flash_main_base_station`

#### Project History
- [RF](/RF): Code for interacting with the RF chips and transmitting data between a TX and an RX chip.
- [camera](/camera): Code for interacting with the ArduCAM camera and capturing images.
- [camera_screen](/camera_screen): Code for rendering camera images on the screen on a single microcontroller (no RF chips). Builds on the work in `display` and `camera`.
- [display](/display): Code for decompressing a JPEG image and rendering it on the screen (no RF chips, no camera). Builds on the work in `screen`.
- [screen](/screen): Code for interacting with the screen and rendering raw RGB565 images on it (no RF chips, no camera, no JPEG compression).

#### Supporting Directories
- [CubeMx_Projects](/CubeMx_Projects): CUBEMX project files for configuring GPIOs and peripherals on the STM32F072.
- [Drivers](/Drivers): Helper files for the STM32F072.
- [docs](/docs): Resources used by this README (images, datasheets, etc.).

## Camera
// TODO

## RF Communication
  - This project uses two STM32 boards for transmitting (TX) and receiving (RX) image data via nRF24L01+ RF modules.
  - Each STM32 board communicates with its nRF24L01+ module over SPI (SCK, MOSI, MISO, CSN, CE, IRQ).
  - The code for the RF communication is demonstrated at [/RF](/RF/), and they are divided as [/RF/RX](/RF/RX) and [/RF/TX](/RF/TX) for receiving and transmitting.

  ### TX STM32 ↔ nRF24L01+ (TX)
  | STM32 Pin | nRF24 Pin | Description |
  |-----------|-----------|-------------|
  | PB13 | SCK | SPI clock |
  | PB14 | MISO | SPI data: nRF24 → STM32 |
  | PC3 | MOSI | SPI data: STM32 → nRF24 |
  | PB12 | CSN | Chip select (active low) |
  | PB1 | CE | TX/RX mode control |
  | PA0 | IRQ | Interrupt: tx done / error |
  | 3.3V | VCC | Power |
  | GND | GND | Ground |

  ### RX STM32 ↔ nRF24L01+ (RX)
  | STM32 Pin | nRF24 Pin | Description |
  |-----------|-----------|-------------|
  | PB13 | SCK | SPI clock |
  | PB14 | MISO | SPI data: nRF24 → STM32 |
  | PC3 | MOSI | SPI data: STM32 → nRF24 |
  | PB10 | CSN | Chip select (active low) |
  | PB11 | CE | TX/RX mode control |
  | PA0 | IRQ | Interrupt: data received |
  | 3.3V | VCC | Power |
  | GND | GND | Ground |

## JPEG Decompression
The base station uses the TJpegDec library to decompress incoming JPEG image data and render it to the screen.

**Why JPEG?**
A raw 320×240 RGB565 image requires ~150 KB of RAM. The STM32F072 has only 16 KB total, so storing or processing a raw frame is impossible. The ArduCAM OV2640 is configured to output JPEG-compressed images instead, which fit within the 10 KB image buffer shared between the camera and base station modules.
Library: TJpegDec
TJpegDec is a JPEG decompressor specifically designed for resource-constrained embedded systems. Key properties relevant to this project:

Decompresses in small MCU (Minimum Coded Unit) blocks rather than all at once, so only a small working buffer needs to be live at any time
Configurable workspace size — tuned here to fit alongside the 10 KB image buffer within 16 KB RAM
Output callback-based: each decoded block is passed to a user-defined function, which performs an endian swap and writes the block to the screen over SPI1

**Memory Constraints & Linker Script Changes**
To fit both the 10 KB image buffer and TJpegDec's workspace into 16 KB RAM:
  -Heap was removed from the linker script (STM32F072XB.ld), since dynamic allocation isn't used and heap competes directly with static buffers
  -Data structures were modified to use bitmasking to pack boolean flags into single bits rather than full bytes

**Endian Swap**
The STM32F072 and the JPEG data are little-endian, but the ILI9341 screen expects big-endian RGB565 values. Each MCU block output by TJpegDec must be byte-swapped before transmission to the screen. This is accelerated using the ARM REV16 instruction, which swaps bytes within each 16-bit halfword — processing 4 bytes per clock cycle.
**Performance**
JPEG decompression is computationally expensive on the STM32F072. At its maximum core clock of 48 MHz, decompressing a single 320×240 JPEG frame takes approximately 5 seconds, limiting the system's effective refresh rate to ~0.2 Hz. A more powerful MCU would be required to achieve real-time video.


## Screen
The screen purchased contains three devices in one: the screen itself that uses the ILI9341 driver chip, a touchscreen that uses the TSC2007 controller, and an SD card interface. The touchscreen and SD card are currently unused in this project.

The screen is primarily intended for use with Arduino boards that operate at 5V; however, we found the PCB schematic for this screen and reviewed the datasheets of the ICs used on the screen PCB and verified that this screen, as long as it's powered by 5V, can accept 3.3V on its inputs and won't output 5V on any of its outputs as long as the reference is set to 3.3V. This means this screen can be powered by the Discovery's 5V port and integrated with its 3.3V logic. (The motivation for this is that this screen costs less than competing screens but has more functionality).

This screen received a fairly major upgrade at the end of 2023. Unfortunately, the [official documentation](https://learn.adafruit.com/adafruit-2-8-tft-touch-shield-v2/downloads) wasn't updated and the pins weren't labeled on the PCB, so getting this screen to work required trial-and-error experimentation and some reverse-engineering. We have therefore included correct documentation for this screen below.

### Pinout
In the figure below, green labels indicate signals that are used by this project. Red labels aren't currently used but were included for future reference. Additionally, labels that begin with an asterisk indicate pins that can be used after their corresponding solder jumpers are closed. Each pin from the figure is described below.

- **<ins>General Signals</ins>**
  - **3.3V:** Used as a reference signal for inputs to the microcontroller and the screen's level shifter. (The level shifter supports 'shifting' signals from 3.3V to 3.3V).
  - **RST:** Used as a hard reset for the screen and touchscreen driver chips. The screen also features a button connected to this pin without any resistors, so the microcontroller GPIO connected to this pin should be configured as an open-drain with a pull-up resistor.
  - **5V:** The primary power supply for the screen PCB.
  - **GND:** All of the GND pins are connected, so any or all can be utilized.
  - **MISO:** Master-In Slave-Out for the SPI protocol. This connects to both the screen and the SD card, but it's realistically only used for the SD card - there's not a lot of use-case for reading data from the screen.
  - **MOSI:** Master-Out Slave-In for the SPI protocol. This connects to both the screen and the SD card.
  - **SCK:** The clock for SPI communication.
- **<ins>Screen (TFT)</ins>**
  - **TFT CS:** The screen's Chip Select pin for the SPI protocol. MISO and MOSI are both connected to multiple devices and SPI doesn't use addresses, so this pin is pulled low to signal to a chip that it's being addressed. This pin has an external resistor pulling the signal up to 3.3V, so the STM32F072 should configure its GPIO pin for this connection as an open drain with no pull-up or -down resistors.
  - **TFT DC:** Indicates to the screen driver chip whether commands or pixel data are being transmitted.
  - **TFT PWM:** Can be connected to a PWM signal to change the brightness of the screen. The solder jumper for this pin is labeled `LITE`.
- **<ins>Touchscreen (TS)</ins>**
  - **TS SCL:** The I2C clock line for communicating with the touchscreen.
  - **TS SDA:** The I2C data line for communicating with the touchscreen. By default, the touchscreen has I2C address `0x48`.
  - **TS INT:** An interrupt output that's raised when a touch is detected on the touchscreen. The solder jumper for this pin is labeled `TSIRQ`.
- **<ins>SD Card (SD)</ins>**
  - **SD CS:** The SD card's Chip Select for the SPI protocol.

![Screen Pinout](/docs/img/screen_pinout.png "Screen Pinout")

### Wiring Diagram
To wire the screen using the STM32F072's SPI1 interface for the screen demonstration at [/screen](/screen/), use the wiring diagram below.

![Screen Wiring Diagram](/docs/img/screen_connection.png "Screen Wiring Diagram")


### PCB Schematic
The correct PCB schematic for this screen can only be downloaded from the [official documentation](https://learn.adafruit.com/adafruit-2-8-tft-touch-shield-v2/downloads) as an EagleCAD file. For ease of use, this schematic is included below.

#### Pin Headers, Reset Circuitry, Level Shifter, & Touchscreen Driver
![Screen Header Schematic](/docs/img/screen_schematic_headers.png "Screen Header Schematic")

#### Screen
![Screen Schematic](/docs/img/screen_schematic_screen.png "Screen Schematic")

#### SD Card
![Screen SD Card Schematic](/docs/img/screen_schematic_sdcard.png "Screen SD Card Schematic")

#### Power
![Screen Power Schematic](/docs/img/screen_schematic_power.png "Screen Power Schematic")

#### STEMMA QT Connector
![Screen STEMMA QT Schematic](/docs/img/screen_schematic_stemmaqt.png "Screen STEMMA QT Schematic")

## Milestones
- **Milestone 1 (3/27) Camera Initialization and Data Capture:**
  - Initialize the ArduCAM OV2640 over SPI/I2C (SCCB) and capture a single frame (at least). Verify data output by reading frame size and raw bytes over UART output. Understand JPEG output from ArduCAM to see if any preprocessing is needed prior to input to the JPEG decompression algorithm. Understand how output from the JPEG decompression algorithm is formatted so that it can be written to the screen.
- **Milestone 2 (4/3) Display Interface and RF Basic Communication:**
  - Initialize and display test images on screen using bus interfaces. Establish communication between two RF modules by verifying packets can be sent and received between two micro controllers using the RF modules.
- **Milestone 3 (4/10) Timer and Interrupts Driven Camera Capture:**
  - Implement timer-based frame capture using timers and interrupts. Camera captures frames at a consistent rate. 
- **Milestone 4 (4/17) Live Camera Image on Screen:**
  - Flash JPEG frames from camera to screen using a single microcontroller. The screen will display live camera feed.
- **Milestone 5 (4/24) Wireless Image Transmission:**
  - Transmit captured camera frames wirelessly between two microcontrollers using RF modules and communication protocol. The receiving microcontroller will display feed on the screen. Fallback: use I2C if RF integration is not finished.
- **Milestone 6 + Final Submission (5/1):**
  - Full System Integration, Testing, and Documentation: Completed system working reliably, including camera capture, wireless transmit, and remote screen display. Hardware and software are documented in GitHub. Final code to be cleaned and polished before presentation. 
