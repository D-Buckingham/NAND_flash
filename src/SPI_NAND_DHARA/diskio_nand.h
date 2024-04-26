/**
 * @file diskio_nand.h
 * @brief Adapter to integrate the Dhara NAND Flash Translation Layer (FTL) with the FAT file system in Zephyr RTOS.
 *
 * This header file defines the disk access interface for NAND flash devices,
 * allowing integration of NAND storage as a disk with file system support in Zephyr.
 * Author: [Denis Buckingham]
 * Date: [10.03.2024]
 */


#ifndef DISKIO_NAND_H
#define DISKIO_NAND_H

#include <zephyr/types.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include "zephyr/storage/disk_access.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initializes the disk for NAND flash.
 *
 * @param disk Pointer to the disk_info structure representing the NAND disk.
 * @return 0 on success, negative errno code on failure.
 */
static int nand_disk_access_init(struct disk_info *disk);

/**
 * Gets the status of the NAND flash disk.
 *
 * @param disk Pointer to the disk_info structure representing the NAND disk.
 * @return DISK_STATUS_OK if the disk is ready, other DISK_STATUS_* codes on failure.
 */
static int nand_disk_access_status(struct disk_info *disk);

/**
 * Reads data from the NAND flash disk.
 *
 * @param disk Pointer to the disk_info structure representing the NAND disk.
 * @param data_buf Buffer to store read data.
 * @param start_sector Start sector number from which to start reading.
 * @param num_sector Number of sectors to read.
 * @return 0 on success, negative errno code on failure.
 */
static int nand_disk_access_read(struct disk_info *disk, uint8_t *data_buf,
                          uint32_t start_sector, uint32_t num_sector);

/**
 * Writes data to the NAND flash disk.
 *
 * @param disk Pointer to the disk_info structure representing the NAND disk.
 * @param data_buf Buffer containing the data to write.
 * @param start_sector Start sector number at which to start writing.
 * @param num_sector Number of sectors to write.
 * @return 0 on success, negative errno code on failure.
 */
static int nand_disk_access_write(struct disk_info *disk, const uint8_t *data_buf,
                           uint32_t start_sector, uint32_t num_sector);

/**
 * Executes IOCTL commands on the NAND flash disk.
 *
 * @param disk Pointer to the disk_info structure representing the NAND disk.
 * @param cmd IOCTL command to execute.
 * @param buff Buffer to transfer data for some IOCTL commands.
 * @return 0 on success, negative errno code on failure.
 */
static int nand_disk_access_ioctl(struct disk_info *disk, uint8_t cmd, void *buff);

/**
 * @brief Initializes and registers the NAND disk.
 *
 * This function sets up the disk information structure and registers
 * the NAND disk with the disk access layer of Zephyr. This registration
 * is necessary for the system to recognize and interact with the disk.
 *
 * @return 0 on success, negative error code on failure.
 */
int disk_nand_init(void);

/**
 * @brief Unregisters and deinitializes the NAND disk.
 *
 * This function unregisters the NAND disk from the disk access layer,
 * effectively making the system no longer recognize it as a valid disk.
 * This is useful for cleanly shutting down or reconfiguring the system.
 *
 * @return 0 on success, negative error code on failure.
 */
int disk_nand_uninit(void);



#ifdef __cplusplus
}
#endif

#endif /* DISKIO_NAND_H */