/**
 * @file spi_nand_oper.h
 * @brief Configuring the 913-S5F14G04SND10LIN NAND flash
 *
 * This file establishes the SPI communication and stores the predefined commands to interfere 
 * with the 913-S5F14G04SND10LIN NAND flash
 * Author: [Denis Buckingham]
 * Date: [10.03.2024]
 */


#ifndef SPI_NAND_OPER_H
#define SPI_NAND_OPER_H

#pragma once

#include <zephyr/kernel.h>
#include <Zephyr/device.h>
#include <Zephyr/drivers/spi.h>
#include <zephyr/devicetree.h>
#include <Zephyr/sys/util.h>

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



#define CMD_WIRTE_DISABLE   0x04
#define CMD_WRITE_ENABLE    0x06
#define CMD_ERASE_BLOCK     0xD8
#define CMD_PROGRAM_LOAD    0x02
#define CMD_PROGRAM_LOAD_X4 0x32
#define CMD_PROGRAM_EXECUTE 0x10
#define CMD_PROGRAM_LOAD_RAND 0x84
#define CMD_PROGRAM_LOAD_RAND_X4 0xC4
#define CMD_PAGE_READ       0x13
#define CMD_READ_FAST       0x0B
#define CMD_READ_X2         0x3B
#define CMD_READ_X4         0x6B
#define CMD_READ_ID         0x9F

#define CMD_SET_REGISTER    0x1F //commands are used to monitor the device status and alter the device behavior, check this!! TODO
#define CMD_READ_REGISTER   0x0F//The content of status register can be read by issuing the GET FEATURE (0FH) command, followed by the status register address C0H.
#define CMD_Reset           0xFF

#define DEVICE_ADDR_READ    0x01
#define MANUFACTURER_ADDR_READ  0x00



#define REG_STATUS          0xC0
#define REG_PROTECT         0xA0 //block lock
#define REG_CONFIG          0xB0 //otp


#define STAT_BUSY           (1 << 0)
#define STAT_WRITE_ENABLED  (1 << 1)
#define STAT_ERASE_FAILED   (1 << 2)
#define STAT_PROGRAM_FAILED (1 << 3)
#define STAT_ECC0           (1 << 4)
#define STAT_ECC1           (1 << 5)

// Commands, registers, and status flags definitions

/**
 * @brief Initialize the SPI device for NAND operations.
 *
 * This function initializes an SPI device based on the device tree specification.
 * It verifies if the SPI device is ready to be used for communication. In case
 * the device is not ready, it logs an error message.
 *
 * @note Replace `spidev` with the actual node label of your SPI device in the
 *       device tree.
 *
 * @return A spi_dt_spec structure representing the initialized SPI device.
 */
const struct spi_dt_spec spi_nand_init(void);

/**
 * @brief Execute a transaction over SPI to a NAND device.
 *
 * This function prepares and executes a transaction based on the provided
 * transaction parameters including command, address, data to be written
 * or read, and dummy bytes.
 *
 * @param dev Device SPI configuration data obtained from devicetree.
 * @param transaction Transaction parameters including command, address, and data.
 * @return 0 on success, negative errno error code otherwise.
 */
int spi_nand_execute_transaction(const struct spi_dt_spec *dev, spi_nand_transaction_t *transaction);

/**
 * @brief Read a register from the SPI NAND device.
 *
 * @param dev Device SPI configuration data obtained from devicetree.
 * @param reg Register address to read from.
 * @param val Pointer to variable where read value will be stored.
 * @return 0 on success, negative error code otherwise.
 */
int spi_nand_read_register(const struct spi_dt_spec *dev, uint8_t reg, uint8_t *val);

/**
 * @brief Write to a register on the SPI NAND device.
 *
 * @param dev Device SPI configuration data obtained from devicetree.
 * @param reg Register address to write to.
 * @param val Value to write to the register.
 * @return 0 on success, negative error code otherwise.
 */
int spi_nand_write_register(const struct spi_dt_spec *dev, uint8_t reg, uint8_t val);

/**
 * @brief Enable writing on the SPI NAND device.
 *
 * This function must be called before any write operation.
 *
 * @param dev Device SPI configuration data obtained from devicetree.
 * @return 0 on success, negative error code otherwise.
 */
int spi_nand_write_enable(const struct spi_dt_spec *dev);

/**
 * @brief Initiate page read operation to cache.
 *
 * Reads the specified page from NAND to the device's internal cache.
 *
 * @param dev Device SPI configuration data obtained from devicetree.
 * @param page Page number to read. 24-bit address consists of 7 dummy bits and 17 page/block address bits
 * @return 0 on success, negative error code otherwise.
 */
int spi_nand_read_page(const struct spi_dt_spec *dev, uint32_t page);

/**
 * @brief Read data from the SPI NAND device's cache.
 *
 * Assumes the data has been previously loaded into cache using
 * spi_nand_read_page().
 *
 * @param dev Device SPI configuration data obtained from devicetree.
 * @param column Start column (byte) in the page from where to start reading.
 * @param length Number of bytes to read.
 * @return 0 on success, negative error code otherwise.
 */
int spi_nand_read(const struct spi_dt_spec *dev, uint8_t *data, uint16_t column, uint16_t length);

/**
 * @brief Execute a program operation.
 *
 * Commits the data previously loaded into the device's cache to the NAND array.
 *
 * @param dev Device SPI configuration data obtained from devicetree.
 * @param page Page number to program.
 * @return 0 on success, negative error code otherwise.
 */
int spi_nand_program_execute(const struct spi_dt_spec *dev, uint32_t page);

/**
 * @brief Load data into the SPI NAND device's cache.
 *
 * @param dev Device SPI configuration data obtained from devicetree.
 * @param data Pointer to data to be written to cache.
 * @param column Start column (byte) in the page from where to start writing.
 * @param length Number of bytes to write.
 * @return 0 on success, negative error code otherwise.
 */
int spi_nand_program_load(const struct spi_dt_spec *dev, const uint8_t *data, uint16_t column, uint16_t length);

/**
 * @brief Erase a block on the SPI NAND device.
 *
 * @param dev Device SPI configuration data obtained from devicetree.
 * @param page Page number within the block to be erased.
 * @return 0 on success, negative error code otherwise.
 */
int spi_nand_erase_block(const struct spi_dt_spec *dev, uint32_t page);


/**
 * @brief Read out device ID
 * 
 * @param dev Device SPI configuration data obtained from devicetree.
 * @param device_id Device ID
*/
int spi_nand_device_id(const struct spi_dt_spec *dev, uint8_t *device_id);


/**
 * @brief Test function to validate the SPI communication. Reads out the device ID
 * 
 * @param dev Device SPI configuration data obtained from devicetree. 
*/
int spi_nand_test(const struct spi_dt_spec *dev);




#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
}
#endif

#endif //SPI_NAND_OPER_H
