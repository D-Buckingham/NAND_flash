#ifndef NAND_TOP_LAYER_H
#define NAND_TOP_LAYER_H


#include <stdint.h>
#include "map.h"

#include <zephyr/kernel.h>
#include <zephyr/drivers/spi.h>


#ifdef __cplusplus
extern "C" {
#endif

#define INVALID_PAGE 0xFFFF


/** @brief Structure to describe how to configure the nand access layer.
 @note The spi_device_handle_t must be initialized with the flag SPI_DEVICE_HALFDUPLEX
*/
typedef struct spi_nand_flash_config_t{
    const struct device *spi_dev;//spi_device_handle_t device_handle;       ///< SPI Device for this nand chip.
    uint8_t gc_factor;                       ///< The gc factor controls the number of blocks to spare block ratio.
    ///< Lower values will reduce the available space but increase performance
}spi_nand_flash_config_t;





typedef struct spi_nand_flash_device_t{
    spi_nand_flash_config_t config;
    uint32_t block_size;
    uint32_t page_size;
    uint32_t num_blocks;
    struct dhara_map dhara_map;
    struct dhara_nand dhara_nand;
    uint8_t *work_buffer;
    uint32_t read_page_delay_us;
    uint32_t erase_block_delay_us;
    uint32_t program_page_delay_us;
    //SemaphoreHandle_t mutex;
    struct k_sem mutex;  // Zephyr semaphore
}spi_nand_flash_device_t;


//according to best practice
//typedef struct spi_nand_flash_config_t {} spi_nand_flash_config_t;
//typedef struct spi_nand_flash_device_t {} spi_nand_flash_device_t;

//defined in the glue between spi_nand_oper and dhara
int wait_for_ready(const struct device *device, uint32_t expected_operation_time_us, uint8_t *status_out);





/** @brief Initialise SPI nand flash chip interface.
 *
 * This function must be called before calling any other API functions for the nand flash.
 *
 * @param config Pointer to SPI nand flash config structure.
 * @param[out] handle The handle to the SPI nand flash chip is returned in this variable.
 * @return ESP_OK on success, or a flash error code if the initialisation failed.
 */
int spi_nand_flash_init_device(spi_nand_flash_config_t *config, spi_nand_flash_device_t **handle);

/** @brief Read a sector from the nand flash.
 *
 * @param handle The handle to the SPI nand flash chip.
 * @param[out] buffer The output buffer to put the read data into.
 * @param sector_id The id of the sector to read.
 * @return ESP_OK on success, or a flash error code if the read failed.
 */
int spi_nand_flash_read_sector(spi_nand_flash_device_t *handle, uint8_t *buffer, uint16_t sector_id);

/** @brief Write a sector to the nand flash.
 *
 * @param handle The handle to the SPI nand flash chip.
 * @param[out] buffer The input buffer containing the data to write.
 * @param sector_id The id of the sector to write.
 * @return ESP_OK on success, or a flash error code if the write failed.
 */
int spi_nand_flash_write_sector(spi_nand_flash_device_t *handle, const uint8_t *buffer, uint16_t sector_id);

/** @brief Synchronizes any cache to the device.
 *
 * After this method is called, the nand flash chip should be synchronized with the results of any previous read/writes.
 *
 * @param handle The handle to the SPI nand flash chip.
 * @return ESP_OK on success, or a flash error code if the synchronization failed.
 */
int spi_nand_flash_sync(spi_nand_flash_device_t *handle);

/** @brief Retrieve the number of sectors available.
 *
 * @param handle The handle to the SPI nand flash chip.
 * @param[out] number_of_sectors A pointer of where to put the return value
 * @return ESP_OK on success, or a flash error code if the operation failed.
 */
int spi_nand_flash_get_capacity(spi_nand_flash_device_t *handle, uint16_t *number_of_sectors);

/** @brief Retrieve the size of each sector.
 *
 * @param handle The handle to the SPI nand flash chip.
 * @param[out] number_of_sectors A pointer of where to put the return value
 * @return ESP_OK on success, or a flash error code if the operation failed.
 */
int spi_nand_flash_get_sector_size(spi_nand_flash_device_t *handle, uint16_t *sector_size);

/** @brief Erases the entire chip, invalidating any data on the chip.
 *
 * @param handle The handle to the SPI nand flash chip.
 * @return ESP_OK on success, or a flash error code if the erase failed.
 */
int spi_nand_erase_chip(spi_nand_flash_device_t *handle);

/** @brief De-initialize the handle, releasing any resources reserved.
 *
 * @param handle The handle to the SPI nand flash chip.
 * @return ESP_OK on success, or a flash error code if the de-initialization failed.
 */
int spi_nand_flash_deinit_device(spi_nand_flash_device_t *handle);


#ifdef __cplusplus
}
#endif

#endif // NAND_TOP_LAYER_H