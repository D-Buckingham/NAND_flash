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

#endif //FVS_TEST