/*
 * NAND_flash.h
 *
 * This header file defines a set of functions for interacting with a FAT file system
 * using the Zephyr OS File System API. The functions abstract common file operations,
 * including initialization of the file system, opening, reading, writing, and closing
 * files, as well as manipulating directories. The aim is to provide a simplified interface
 * for applications to perform file operations on a FAT file system..
 *
 *
 *
 * Author: [Denis Buckingham]
 * Date: [10.03.2024]
 */


void mount_nand_fs(void);