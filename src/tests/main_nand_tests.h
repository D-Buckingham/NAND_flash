/**
 * @file main_nand_tests.h
 * @brief Testing functions to test the entire implementation and retrieve information about
 * the underlying processes while manipulating the NAND flash
 *
 * 
 * Author: [Denis Buckingham]
 * Date: [7.05.2024]
 */

#ifndef TEST_SPI_NAND_TOP_LAYER_H
#define TEST_SPI_NAND_TOP_LAYER_H

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

/** 
 * @brief Tests whether the SPI NAND device is connected and initialized correctly.
 * 
 * This function performs various checks to ensure that the SPI NAND device
 * is correctly connected and working at multiple levels, including:
 * - Checking if the SPI bus device is ready.
 * - Validating the device and manufacturer IDs.
 * - Retrieving the sector size using Dhara map layer.
 * - Checking if the disk is properly connected via disk access layer.
 * - Confirming the NAND filesystem mount status.
 * 
 * @return 0 if the device is properly connected and initialized, -1 otherwise.
 */
int top_device_connected(void);

/**
 * @brief Test if a folder can be created on the NAND filesystem.
 * 
 * @return 0 if successful, -1 otherwise.
 */
int test_create_folder(void);

/**
 * @brief Test if a file can be created on the NAND filesystem.
 * 
 * It creates, stores and reads out the file afterwards
 * 
 * @return 0 if successful, -1 otherwise.
 */
int test_create_file(void);


/**
 * @brief Test storing a large file that spans multiple blocks on the NAND filesystem.
 * 
 * @return 0 if successful, -1 otherwise.
 */
int test_store_large_file(void);

/**
 * @brief Test appending data to a large file and inspect the changes.
 * 
 * @return 0 if successful, -1 otherwise.
 */
int test_append_data_large_file(void);

/**
 * @brief Test changing the data of a file and analyze the process on the NAND device.
 * 
 * @return 0 if successful, -1 otherwise.
 */
int test_change_file_data(void);

/**
 * @brief Test deleting a file from the NAND filesystem.
 * 
 * @return 0 if successful, -1 otherwise.
 */
int test_delete_file(void);

/**
 * @brief Test writing to one-eighth of the flash memory.
 * 
 * @return 0 if successful, -1 otherwise.
 */
int test_write_one_eighth_flash(void);


/**
 * @brief Main function that calls all other test functions.
 * 
 * Calls the following functions sequentially:
 * - top_device_connected()
 * - test_create_folder()
 * - test_create_file()
 * - test_read_file()
 * - test_store_large_file()
 * - test_append_data_large_file()
 * - test_change_file_data()
 * - test_delete_file()
 * - test_write_one_eighth_flash()
 * - test_store_multiple_small_files()
 * 
 * @return 0 if all tests passed, -1 otherwise.
 */
int test_all_main_nand_tests(void);

#endif // TEST_SPI_NAND_TOP_LAYER_H
