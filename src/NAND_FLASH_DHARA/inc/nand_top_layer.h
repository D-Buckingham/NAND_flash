/**
 * @file nand_top_layer.h
 * @brief Configuring the NAND flash
 *
 * Author: [Denis Buckingham]
 * Date: [10.03.2024]
 */

#ifndef NAND_TOP_LAYER_H
#define NAND_TOP_LAYER_H


#include <stdint.h>
#include "../dhara/dhara/map.h"

#include <zephyr/kernel.h>


#ifdef __cplusplus
extern "C" {
#endif

#define INVALID_PAGE 0xFFFF

#define ROM_WAIT_THRESHOLD_US 1000



typedef struct nand_flash_device_t{
    uint8_t gc_factor;
    uint32_t block_size;
    uint32_t page_size;
    uint32_t num_blocks;
    struct dhara_map dhara_map;
    struct dhara_nand dhara_nand;
    uint8_t *work_buffer;
    struct k_sem mutex;  // Zephyr semaphore
}nand_flash_device_t;


//according to best practice?
//typedef struct nand_flash_config_t {} nand_flash_config_t;
//typedef struct nand_flash_device_t {} nand_flash_device_t;

//shared externally
extern nand_flash_device_t *device_handle;





/** @brief waiting for finished transaction
 *
 * defined in the nand.c between nand_oper and dhara
 * used in the top layer and nand.c
 *
 * @param device Device configuration data obtained from devicetree.
 * @param expected_operation_time_us
 * @param[out] status_out status register content of current transaction
 * @return 0 on success, -1 if the read out of the register failed.
 */
int wait_for_ready(uint8_t *status_out);





/** @brief Initialise nand flash chip interface.
 *
 * This function must be called before calling any other API functions for the nand flash.
 *
 * @param config Pointer to nand flash config structure.
 * @param[out] handle The handle to the nand flash chip is returned in this variable.
 * @return  on success, or -1 and a commented error code if the initialisation failed.
 */
int nand_flash_init_device(nand_flash_device_t **handle);

/** @brief Read a sector from the nand flash.
 *
 * @param handle The handle to the nand flash chip.
 * @param[out] buffer The output buffer to put the read data into.
 * @param sector_id The id of the sector to read.
 * @return 0 on success, or -1 if the read failed.
 */
int nand_flash_read_sector(nand_flash_device_t *handle, uint8_t *buffer, uint16_t sector_id);

/** @brief Write a sector to the nand flash.
 *
 * @param handle The handle to the nand flash chip.
 * @param[out] buffer The input buffer containing the data to write.
 * @param sector_id The id of the sector to write.
 * @return 0 on success, or -1 if the write failed.
 */
int nand_flash_write_sector(nand_flash_device_t *handle, const uint8_t *buffer, uint16_t sector_id);

/** @brief Synchronizes any cache to the device.
 *
 * After this method is called, the nand flash chip should be synchronized with the results of any previous read/writes.
 *
 * @param handle The handle to the nand flash chip.
 * @return 0 on success, or -1 if the synchronization failed.
 */
int nand_flash_sync(nand_flash_device_t *handle);

/** @brief Retrieve the number of sectors available.
 *
 * @param handle The handle to the nand flash chip.
 * @param[out] number_of_sectors A pointer of where to put the return value
 * @return 0 on success, or -1 if the operation failed.
 */
int nand_flash_get_capacity(nand_flash_device_t *handle, uint32_t *number_of_sectors);

/** @brief Retrieve the size of each sector.
 *
 * @param handle The handle to the nand flash chip.
 * @param[out] number_of_sectors A pointer of where to put the return value
 * @return 0 on success, or -1 if the operation failed.
 */
int nand_flash_get_sector_size(nand_flash_device_t *handle, uint32_t *sector_size);

/** @brief Erases the entire chip, invalidating any data on the chip.
 *
 * @param handle The handle to the nand flash chip.
 * @return 0 on success, or -1 if the erase failed.
 */
int nand_erase_chip(nand_flash_device_t *handle);

/** @brief De-initialize the handle, releasing any resources reserved.
 *
 * @param handle The handle to the nand flash chip.
 * @return 0 on success, or -1 if the de-initialization failed.
 */
int nand_flash_deinit_device(nand_flash_device_t *handle);


#ifdef __cplusplus
}
#endif

#endif // NAND_TOP_LAYER_H