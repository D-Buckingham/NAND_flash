/**
 * @file spi_nand_oper_tests.h
 * @brief Testing functions to test the lowest layer that incorporates the communication 
 * with the flash over SPI 
 *
 * Tests:
 * register read
 * register write
 * page read
 * read (cache)
 * execute program (cache to nand array)
 * erase block
 * read device ID & manufacturer ID
 * 
 * Author: [Denis Buckingham]
 * Date: [18.03.2024]
 */


#ifndef SPI_NAND_OPER_TESTS_H
#define SPI_NAND_OPER_TESTS_H

#pragma once

#include <zephyr/kernel.h>
#include <Zephyr/device.h>
#include <Zephyr/drivers/spi.h>
#include <Zephyr/sys/util.h>
#include <zephyr/devicetree.h>

#include "nand_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Test writing to a register on the SPI NAND device.
 *
 * This function is a placeholder for future implementation of write operations
 * to NAND registers. Currently, it does not perform any operations to avoid accidental
 * modification of device settings.
 * 
 * @param dev Pointer to the SPI device structure.
 * @return Returns 0 on success, or -1 if the device is not ready.
 */
int test_write_register_spi_nand(const struct spi_dt_spec *dev);

/**
 * @brief Test reading a page from SPI NAND to the device's cache.
 *
 * This function demonstrates initiating a read operation for a specific page
 * from the NAND flash memory to its internal cache.
 * 
 * @param dev Pointer to the SPI device structure.
 * @return Returns 0 on success, or -1 if the device is not ready or if the read operation fails.
 */
int test_read_page_spi_nand(const struct spi_dt_spec *dev);

/**
 * @brief Test reading from the device's cache.
 *
 * This function assumes data has already been read into the device's cache
 * and attempts to read from this cache into a buffer for verification.
 * 
 * @param dev Pointer to the SPI device structure.
 * @return Returns 0 on success, or -1 if the device is not ready or if the cache read fails.
 */
int test_read_cache_spi_nand(const struct spi_dt_spec *dev);

/**
 * @brief Test programming data to NAND and executing the program operation.
 *
 * This function demonstrates writing data to the device's cache and then
 * committing this data to the NAND array through a program execute command.
 * 
 * @param dev Pointer to the SPI device structure.
 * @return Returns 0 on success, or -1 if the device is not ready or if the program operation fails.
 */
int test_load_and_execute_program_spi_nand(const struct spi_dt_spec *dev);

/**
 * @brief Test erasing a block in the SPI NAND device.
 *
 * This function initiates an erase operation for a specified block within
 * the NAND flash memory and checks for successful completion.
 * 
 * @param dev Pointer to the SPI device structure.
 * @return Returns 0 on success, or -1 if the device is not ready or if the erase operation fails.
 */
int test_erase_block_spi_nand(const struct spi_dt_spec *dev);

/**
 * @brief Test retrieving device and manufacturer ID from the SPI NAND device.
 *
 * This function reads the device and manufacturer ID from the NAND flash memory
 * to help identify the NAND chip.
 * 
 * @param dev Pointer to the SPI device structure.
 * @return Returns 0 on success, or -1 if the device is not ready or if the ID read operation fails.
 */
int test_IDs_spi_nand(const struct spi_dt_spec *dev);

/**
 * @brief Tests the SPI NAND write and read operation.
 *
 * Performs a test write operation to a specific page and then reads back
 * the written data to verify the operation. It tests the sequence of enabling
 * write, programming a load, executing the program, and reading the page.
 *
 * @param dev Pointer to the SPI device structure.
 */
int test_spi_nand_write_read(const struct spi_dt_spec *dev);

/**
 * @brief Tests the SPI NAND write and read operation on an entire sector.
 *
 * Performs a test write operation to a sector and then reads back
 * the written data to verify the operation. It uses random numbers for this 
 * operations. It tests the sequence of enabling write, programming a load, 
 * executing the program, and reading the page.
 *
 * @param dev Pointer to the SPI device structure.
 */
int test_spi_nand_sector_write_read(const struct spi_dt_spec *dev);
/**
 * @brief Execute all SPI NAND communicator tests.
 *
 * This function runs a series of tests designed to verify the functionality
 * of the SPI NAND communicator. It tests register writing, page reading to cache,
 * cache reading, loading and executing program operations, block erasing,
 * and device & manufacturer ID fetching.
 * 
 * @param dev Pointer to the SPI device structure.
 * @return Returns 0 on success of all tests, or the error code of the failed test.
 */
int test_SPI_NAND_Communicator_all_tests(const struct spi_dt_spec *dev);

#endif //SPI_NAND_OPER_TESTS_H