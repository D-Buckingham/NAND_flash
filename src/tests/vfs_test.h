/**
 * @file vfs_test.h
 * @brief Testing functions to test the mounting, file system
 *
 * 
 * Author: [Denis Buckingham]
 * Date: [10.05.2024]
 */

#ifndef FVS_TEST
#define FVS_TEST

#pragma once


/**
 * @brief Test the NAND Flash filesystem operations.
 *
 * Mounts the NAND filesystem, writes and reads test files, and
 * finally unmounts the filesystem.
 *
 * @return 0 on success, negative error code on failure.
 */
int test_NAND_flash(void);

/**
 * @brief List the contents of a directory.
 *
 * This function prints the contents of a directory at the given path.
 *
 * @param path Path to the directory to list.
 * @return 0 on success, negative error code on failure.
 */
int lsdir(const char *path);

/**
 * @brief Erase the specified NAND flash area.
 *
 * This function erases a given partition in the NAND flash.
 *
 * @param id The ID of the flash area to erase.
 * @return 0 on success, negative error code on failure.
 */
int nand_flash_erase(unsigned int id);



/**
 * @brief Increment the value inside a file.
 *
 * This function reads a value from the specified file, increments it,
 * and writes it back.
 *
 * @param fname The name of the file to adjust.
 * @return 0 on success, negative error code on failure.
 */
int nand_increase_infile_value(char *fname);

/**
 * @brief Increment the test pattern.
 *
 * This function increments the test pattern inside the given buffer.
 *
 * @param p Pointer to the pattern buffer.
 * @param size Size of the pattern buffer.
 * @param inc Increment value.
 */
void incr_pattern(uint8_t *p, uint16_t size, uint8_t inc);

/**
 * @brief Initialize the test pattern.
 *
 * This function initializes the test pattern inside the given buffer.
 *
 * @param p Pointer to the pattern buffer.
 * @param size Size of the pattern buffer.
 */
void init_pattern(uint8_t *p, uint16_t size);

/**
 * @brief Print the test pattern.
 *
 * This function prints the test pattern inside the given buffer.
 *
 * @param p Pointer to the pattern buffer.
 * @param size Size of the pattern buffer.
 */
void print_pattern(uint8_t *p, uint16_t size);


#endif //FVS_TEST