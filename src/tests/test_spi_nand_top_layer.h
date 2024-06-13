/**
 * @file test_spi_nand_top_layer.h
 * @brief Testing functions to test the wrapper to the DHARA flash translation layer and the subsequent ones
 *
 * 
 * Author: [Denis Buckingham]
 * Date: [10.03.2024]
 */

#ifndef TEST_SPI_NAND_TOP_LAYER
#define TEST_SPI_NAND_TOP_LAYER

#pragma once

#include <zephyr/kernel.h>
#include <Zephyr/device.h>
#include <Zephyr/drivers/spi.h>
#include <Zephyr/sys/util.h>
#include <zephyr/devicetree.h>

#include "nand_top_layer.h"

/**
 * Initializes the SPI NAND flash device.
 *
 * @param[out] out_handle Pointer to the device handle pointer where the initialized device info will be stored.
 * @param[in] spi_handle Pointer to the SPI device specification structure.
 */
void setup_nand_flash(spi_nand_flash_device_t **out_handle, const struct spi_dt_spec *spi_handle);


/**
 * Waits for the NAND device to be ready.
 *
 * @param[in] dev Pointer to the SPI device specification structure.
 * @return 0 on success, negative error code on failure.
 */
int wait_and_chill(const struct spi_dt_spec *dev);

/**
 * Test function to setup, erase and deinitialize the NAND flash.
 *
 * @param[in] spi Pointer to the SPI device specification structure.
 * @return 0 on success, negative error code on failure.
 */
int test1_setup_erase_deinit_top_layer(const struct spi_dt_spec *spi);

/**
 * Checks if the buffer contents match the expected pattern.
 *
 * @param[in] seed Seed used to generate the pattern.
 * @param[in] src Pointer to the source buffer.
 * @param[in] count Number of bytes to check.
 * @return 0 on success, negative error code on mismatch.
 */
int check_buffer(uint32_t seed, const uint8_t *src, size_t count);

/**
 * Fills the buffer with a pattern based on the provided seed.
 *
 * @param[in] seed Seed used to generate the pattern.
 * @param[out] dst Pointer to the destination buffer.
 * @param[in] count Number of bytes to fill.
 */
void fill_buffer(uint32_t seed, uint8_t *dst, size_t count);

/**
 * Performs a single write test on the specified flash sectors.
 *
 * @param[in] flash Pointer to the initialized NAND flash device.
 * @param[in] start_sec Starting sector index for the test.
 * @param[in] sec_count Number of sectors to test.
 * @return 0 on success, negative error code on failure.
 */
int do_single_write_test(spi_nand_flash_device_t *flash, uint32_t start_sec, uint16_t sec_count);

/**
 * Conducts a series of write tests on the NAND flash.
 *
 * @param[in] spi Pointer to the SPI device specification structure.
 * @return 0 on success, negative error code on failure.
 */
int test2_writing_tests_top_layer(const struct spi_dt_spec *spi);

/**
 * Tests writing and reading to and from a specific sector using the NAND flash device.
 *
 * @param[in] spi Pointer to the SPI device specification structure.
 * @return 0 on success, negative error code on failure.
 */
int test_struct_handling(const struct spi_dt_spec *spi);

/**
 * Main function to run all tests on the NAND top layer.
 *
 * @param[in] spidev_dt Pointer to the SPI device specification structure.
 * @return 0 on success, negative error code on failure.
 */
int test_nand_top_layer(const struct spi_dt_spec *spidev_dt);



#endif  //TEST_SPI_NAND_TOP_LAYER