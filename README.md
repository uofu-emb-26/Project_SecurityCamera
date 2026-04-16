# Security Camera Project
## Group Members
- Zachary Ward
- Zoey Lee
- Charles Jones
- Sangeun An

## Overview
This project implements an embedded security camera system. The system reads video data off of a camera, transmits that data to another microcontroller, and then displays that data on a screen. The project consists of two  separate modules: the camera module and the base station, which manages the screen. The modules communicate wirelessly using an RF chip and a custom communication protocol. To meet the resource constraints of the STM32F072 microcontrollers, the JPEG compression mode of the camera is utilized along with a microcontroller-optimized JPEG decompression and processing library. This requires two STM32F072 microcontrollers, two RF chips, one camera, and one screen. An overview of these hardware modules is provided below.

![System Overview](/docs/img/system_overview.png "System Overview")

## Hardware
The following hardware is used to implement this project.

|    **Component**    |                  **Part**                 | **Quantity** |  **Price** |                      **Link**                      |
|:-------------------:|:-----------------------------------------:|:------------:|:----------:|:--------------------------------------------------:|
| **Microcontroller** |      STM32F072 Discovery Kit (UM1690)     |       2      |  $11.13 ea | https://estore.st.com/en/stm32f072b-disco-cpn.html |
|      **Camera**     |       Arducam Mini 2MP Plus (OV2640)      |       1      |  $25.99 ea |               https://a.co/d/06XITQKc              |
|      **Screen**     | Adafruit 2.8" 320x240 SPI ILI9341 Display |       1      |  $24.95 ea |        https://www.adafruit.com/product/1651       |
|     **RF Chip**     |                 nRF24L01+                 |       2      | $7.89 4-pk |               https://a.co/d/04dAOcaQ              |

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

- **<ins>RF Communication</ins>**
 - This project uses two STM32 boards for transmitting (TX) and receiving (RX) image data via nRF24L01+ RF modules.
- Each STM32 board communicates with its nRF24L01+ module over SPI (SCK, MOSI, MISO, CSN, CE, IRQ).
- The RX STM32 board forwards the received data to a PC via a USB-UART bridge.

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

  ### RX STM32 ↔ USB-UART Bridge (PC)
  | STM32 Pin | UART Pin | Description |
  |-----------|----------|-------------|
  | PC4 | RX | STM32 → PC (image data out) |
  | PC5 | TX | PC → STM32 (optional control) |
  | GND | GND | Ground |

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

# Microcontroller Ports
The screen and RF modules both communicate over SPI. The camera has two interfaces: SPI for image data, and I2C for control. Each STM32F072 has two SPI interfaces and two I2C interfaces that each can use one of two GPIO pins for their I/O. Notes about each of these options, and what they conflict with on the Discovery board, are included at [/docs/spi_i2c_pins.txt](/docs/spi_i2c_pins.txt).

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
