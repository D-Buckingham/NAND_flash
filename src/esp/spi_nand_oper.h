/*
 * SPDX-FileCopyrightText: 2022 mikkeldamsgaard project
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * SPDX-FileContributor: 2015-2023 Espressif Systems (Shanghai) CO LTD
 */

#pragma once

#include <stdint.h>
#include <esp_err.h>
#include <driver/spi_master.h>

#ifdef __cplusplus
extern "C" {
#endif

struct spi_nand_transaction_t {
    uint8_t command;
    uint8_t address_bytes;
    uint32_t address;
    uint32_t mosi_len;
    const uint8_t *mosi_data;
    uint32_t miso_len;
    uint8_t *miso_data;
    uint32_t dummy_bytes;
};

typedef struct spi_nand_transaction_t spi_nand_transaction_t;

#define CMD_SET_REGISTER    0x1F
#define CMD_READ_REGISTER   0x0F
#define CMD_WRITE_ENABLE    0x06
#define CMD_READ_ID         0x9F
#define CMD_PAGE_READ       0x13
#define CMD_PROGRAM_EXECUTE 0x10
#define CMD_PROGRAM_LOAD    0x84
#define CMD_PROGRAM_LOAD_X4 0x34
#define CMD_READ_FAST       0x0B
#define CMD_READ_X2         0x3B
#define CMD_READ_X4         0x6B
#define CMD_ERASE_BLOCK     0xD8

#define REG_PROTECT         0xA0
#define REG_CONFIG          0xB0
#define REG_STATUS          0xC0

#define STAT_BUSY           1 << 0
#define STAT_WRITE_ENABLED  1 << 1
#define STAT_ERASE_FAILED   1 << 2
#define STAT_PROGRAM_FAILED 1 << 3
#define STAT_ECC0           1 << 4
#define STAT_ECC1           1 << 5


/**
 * @brief Execute a transaction over SPI to a NAND device.
 *
 * This function prepares and executes a transaction based on the provided
 * transaction parameters including command, address, data to be written
 * or read, and dummy bytes.
 *
 * @param dev Device SPI configuration data obtained from devicetree.
 * @param transaction Transaction parameters including command, address, and data.
 * @return 0 on success, negative error code otherwise.
 */
int spi_nand_execute_transaction(struct spi_dt_spec *dev, spi_nand_transaction_t *transaction);

/**
 * @brief Read a register from the SPI NAND device.
 *
 * @param dev Device SPI configuration data obtained from devicetree.
 * @param reg Register address to read from.
 * @param val Pointer to variable where read value will be stored.
 * @return 0 on success, negative error code otherwise.
 */
int spi_nand_read_register(struct spi_dt_spec *dev, uint8_t reg, uint8_t *val);

/**
 * @brief Write to a register on the SPI NAND device.
 *
 * @param dev Device SPI configuration data obtained from devicetree.
 * @param reg Register address to write to.
 * @param val Value to write to the register.
 * @return 0 on success, negative error code otherwise.
 */
int spi_nand_write_register(struct spi_dt_spec *dev, uint8_t reg, uint8_t val);

/**
 * @brief Enable writing on the SPI NAND device.
 *
 * This function must be called before any write operation.
 *
 * @param dev Device SPI configuration data obtained from devicetree.
 * @return 0 on success, negative error code otherwise.
 */
int spi_nand_write_enable(struct spi_dt_spec *dev);

/**
 * @brief Initiate page read operation to cache.
 *
 * Reads the specified page from NAND to the device's internal cache.
 *
 * @param dev Device SPI configuration data obtained from devicetree.
 * @param page Page number to read.
 * @return 0 on success, negative error code otherwise.
 */
int spi_nand_read_page(struct spi_dt_spec *dev, uint32_t page);

/**
 * @brief Read data from the SPI NAND device's cache.
 *
 * Assumes the data has been previously loaded into cache using
 * spi_nand_read_page().
 *
 * @param dev Device SPI configuration data obtained from devicetree.
 * @param data Pointer to buffer where read data will be stored.
 * @param column Start column (byte) in the page from where to start reading.
 * @param length Number of bytes to read.
 * @return 0 on success, negative error code otherwise.
 */
int spi_nand_read(struct spi_dt_spec *dev, uint8_t *data, uint16_t column, uint16_t length);

/**
 * @brief Execute a program operation.
 *
 * Commits the data previously loaded into the device's cache to the NAND array.
 *
 * @param dev Device SPI configuration data obtained from devicetree.
 * @param page Page number to program.
 * @return 0 on success, negative error code otherwise.
 */
int spi_nand_program_execute(struct spi_dt_spec *dev, uint32_t page);

/**
 * @brief Load data into the SPI NAND device's cache.
 *
 * @param dev Device SPI configuration data obtained from devicetree.
 * @param data Pointer to data to be written to cache.
 * @param column Start column (byte) in the page from where to start writing.
 * @param length Number of bytes to write.
 * @return 0 on success, negative error code otherwise.
 */
int spi_nand_program_load(struct spi_dt_spec *dev, const uint8_t *data, uint16_t column, uint16_t length);

/**
 * @brief Erase a block on the SPI NAND device.
 *
 * @param dev Device SPI configuration data obtained from devicetree.
 * @param page Page number within the block to be erased.
 * @return 0 on success, negative error code otherwise.
 */
int spi_nand_erase_block(struct spi_dt_spec *dev, uint32_t page);

#ifdef __cplusplus
}
#endif
