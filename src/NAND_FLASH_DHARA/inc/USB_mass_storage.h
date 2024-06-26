#ifndef USB_MASS_STORAGE_H
#define USB_MASS_STORAGE_H

#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/fs/fs.h>

/**
 * @brief Initializes the USB mass storage functionality.
 * 
 * This function sets up the USB mass storage class (MSC) interface and
 * associates it with the NAND flash memory. It enables the USB interface
 * and registers the necessary callbacks for read and write operations.
 */
int initialize_mass_storage_nand(void);

/**
 * @brief Callback for reading data from the NAND flash.
 * 
 * This function is called when the USB host requests to read data from
 * the mass storage device. It reads the specified number of blocks from
 * the NAND flash starting at the given logical block address (LBA) and
 * stores the data in the provided buffer.
 * 
 * @param lba Logical block address to start reading from.
 * @param blk_cnt Number of blocks to read.
 * @param buf Buffer to store the read data.
 * @return 0 if successful, -EIO otherwise.
 */
int msc_read(uint32_t lba, uint32_t blk_cnt, uint8_t *buf);

/**
 * @brief Callback for writing data to the NAND flash.
 * 
 * This function is called when the USB host requests to write data to
 * the mass storage device. It writes the specified number of blocks to
 * the NAND flash starting at the given logical block address (LBA) from
 * the provided buffer.
 * 
 * @param lba Logical block address to start writing to.
 * @param blk_cnt Number of blocks to write.
 * @param buf Buffer containing the data to be written.
 * @return 0 if successful, -EIO otherwise.
 */
int msc_write(uint32_t lba, uint32_t blk_cnt, const uint8_t *buf);

#endif // USB_MASS_STORAGE_H
