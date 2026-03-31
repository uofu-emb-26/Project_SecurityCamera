# Security Camera Project
## Group Members
- Zachary Ward
- Zoey Lee
- Charles Jones
- Sangeun An

## Overview
This project implements an embedded security camera system. The system reads video data off of a camera, transmits that data to another microcontroller, and then displays that data on a screen. The project consists of two  separate modules: the camera module and the base station, which manages the screen. The modules communicate wirelessly using an RF chip and a custom communication protocol. To meet the resource constraints of the STM32F072 microcontrollers, the JPEG compression mode of the camera is utilized along with a microcontroller-optimized JPEG decompression and processing library. This requires two STM32F072 microcontrollers, two RF chips, one camera, and one screen. An overview of these hardware modules is provided below.

![System Overview](/docs/img/system_overview.png "System Overview")

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
