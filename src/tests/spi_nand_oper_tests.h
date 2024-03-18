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

#ifdef __cplusplus
extern "C" {
#endif




register read
 * register write
 * page read
 * read (cache)
 * execute program (cache to nand array)
 * erase block
 * 
 * 



int test_write_register_spi_nand(const struct device *dev);
int test_read_page_spi_nand(const struct device *dev);
int test_read_cache_spi_nand(const struct device *dev);
int test_execute_program_spi_nand(const struct device *dev);
int test_erase_block_spi_nand(const struct device *dev);




/**
 * @brief Test function to validate the SPI communication. Reads out the device and manufacturer ID
 * 
 * @param dev Device SPI configuration data obtained from devicetree. 
*/
int test_IDs_spi_nand(const struct device *dev);



#endif