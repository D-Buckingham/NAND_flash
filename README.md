# Integration of NAND Flashes in Zephyr with the Flash Translation Layer DHARA


## Table of Contents

1. [Introduction](#introduction)
2. [Prerequisites](#prerequisites)
3. [Setup and Configuration](#setup-and-configuration)
   - [Hardware Setup](#hardware-setup)
   - [Software Setup](#software-setup)
4. [Troubleshooting](#troubleshooting)
5. [References](#references)

## Introduction

This guide provides a comprehensive overview of integrating NAND flash memory in the Zephyr RTOS using the DHARA Flash Translation Layer (FTL). The integration ensures reliable storage management, wear leveling, and error correction, making it suitable for embedded applications requiring non-volatile storage.

## Prerequisites

- **Hardware**: For example the nrf5340dk in combination with an external test bench.
- **Software**:
  - Zephyr RTOS
  - VS Code
  - nrf Connect in VS Code
- **Tools**:
  - Git
  - CMake
  - West
  - A supported compiler (e.g., GCC)

## Setup and Configuration

### Hardware Setup

1. **Connect NAND Flash**: Ensure the NAND flash is correctly connected to the development board according to the following illustration.
2. **Verify Connections**: Double-check the connections, especially the data lines, control signals, and power supply.

![NAND Flash Cable Connection](images/Pinout_NAND.png)

### Software Setup
1. **Set Up Zephyr**:

    Follow the [Zephyr Getting Started Guide](https://docs.zephyrproject.org/latest/getting_started/index.html) to install and set up the Zephyr SDK.
2. **Add the nrf Connect extension in vs code**
3. **Build and flash the repo**
   ```c
   west build -p --board nrf5340dk_nrf5340_cpuapp
   west flash
   ```

Adjustments can either be created in the device tree for the SPI or in the overlay.



## Theory Corner

### NAND Flash Multistack Implementation

### DHARA
