#include <stdlib.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>


#include "error.h"
#include "spi_nand_oper.h"
#include "nand.h"
#include "nand_top_layer.h"
#include "nand_flash_devices.h"

LOG_MODULE_REGISTER(nand_top_layer, CONFIG_LOG_DEFAULT_LEVEL);

/**
 * @brief Initialize a Winbond SPI NAND device.
 *
 * This function initializes a Winbond SPI NAND flash device by reading its
 * device ID and setting up specific timing parameters based on the device ID.
 * *
 * @param dev Pointer to the SPI NAND device structure.
 *
 * @retval 0 If the initialization is successful.
 * @retval -1 If there was an error during device ID retrieval or if the device ID
 *         is not recognized as a supported Winbond device.
 */
static int spi_nand_winbond_init(spi_nand_flash_device_t *dev)
{
    uint8_t device_id_buf[2];
    spi_nand_transaction_t t = {
        .command = CMD_READ_ID,
        .dummy_bytes = 2,
        .miso_len = 2,
        .miso_data = device_id_buf
    };
    int err = spi_nand_execute_transaction(dev->config.spi_dev, &t);
    if (err != 0) {
        LOG_ERR("Failed to receive device ID, error: %d", err);
        return -1;
    }

    uint16_t device_id = (device_id_buf[0] << 8) + device_id_buf[1];
    dev->read_page_delay_us = 10;
    dev->erase_block_delay_us = 2500;
    dev->program_page_delay_us = 320;

    switch (device_id) {
    case WINBOND_DI_AA20:
    case WINBOND_DI_BA20:
        dev->dhara_nand.num_blocks = 512;
        break;
    case WINBOND_DI_AA21:
    case WINBOND_DI_BA21:
    case WINBOND_DI_BC21:
        dev->dhara_nand.num_blocks = 1024;
        break;
    default:
        LOG_ERR("Invalid Winbond device ID: %u", device_id);
        return -1;
    }
    return 0;
}

//We use the S5F14G04SND, but it should recognize the flash itself

/**
 * @brief Initializes a NAND device based on its Alliance Memory device ID.
 *
 * This function reads the device ID using a SPI transaction, then sets
 * device-specific timing parameters and the number of blocks based on the
 * device ID. It supports multiple Alliance Memory NAND flash devices by
 * adjusting configuration parameters accordingly.
 *
 * @param dev Pointer to the SPI NAND flash device structure.
 * @return 0 on success, -1 on error with a logged error message.
 */

static int spi_nand_alliance_init(spi_nand_flash_device_t *dev)
{
    int err;
    uint8_t device_id;
    spi_nand_transaction_t t = {
        .command = CMD_READ_ID,
        .address = 1,
        .address_bytes = 1,
        .dummy_bytes = 1,
        .miso_len = 1,
        .miso_data = &device_id
    };
    err = spi_nand_execute_transaction(dev->config.spi_dev, &t);
    if (err != 0) {
        LOG_ERR("Failed to read Alliance device ID, error: %d", err);
        return -1;
    }
    dev->erase_block_delay_us = 3000;
    dev->program_page_delay_us = 630;
    switch (device_id) {
    case ALLIANCE_DI_25: //AS5F31G04SND-08LIN
        dev->dhara_nand.num_blocks = 1024;
        dev->read_page_delay_us = 60;
        break;
    case ALLIANCE_DI_2E: //AS5F32G04SND-08LIN
    case ALLIANCE_DI_8E: //AS5F12G04SND-10LIN
        dev->dhara_nand.num_blocks = 2048;
        dev->read_page_delay_us = 60;
        break;
    case ALLIANCE_DI_2F: //AS5F34G04SND-08LIN
    case ALLIANCE_DI_8F: //AS5F14G04SND-10LIN ==> Current implementation
        dev->dhara_nand.num_blocks = 4096;
        dev->read_page_delay_us = 60;
        break;
    case ALLIANCE_DI_2D: //AS5F38G04SND-08LIN
    case ALLIANCE_DI_8D: //AS5F18G04SND-10LIN
        dev->dhara_nand.log2_page_size = 12; // 4k pages
        dev->dhara_nand.num_blocks = 4096;
        dev->read_page_delay_us = 130; // somewhat slower reads
        break;
    default:
        LOG_ERR("Invalid Alliance device ID: 0x%X", device_id);
            return -1;
    }
    return 0;
}

/**
 * @brief Initializes a GigaDevice SPI NAND flash based on its device ID.
 *
 * This function reads the device ID of a GigaDevice SPI NAND flash and configures
 * various operational parameters based on the detected device. It sets up the
 * number of blocks, read page delay, erase block delay, and program page delay
 * based on the specific device variant identified by the device ID.
 *
 * @param dev Pointer to the spi_nand_flash_device_t structure representing the SPI NAND device.
 * @return 0 on successful initialization and configuration, -1 on failure with an error logged.
 */
static int spi_nand_gigadevice_init(spi_nand_flash_device_t *dev)
{
    uint8_t device_id;
    int err;
    spi_nand_transaction_t t = {
        .command = CMD_READ_ID,
        .dummy_bytes = 2,
        .miso_len = 1,
        .miso_data = &device_id
    };
    err = spi_nand_execute_transaction(dev->config.spi_dev, &t);
    if (err != 0) {
        LOG_ERR("Failed to read gigadevice device ID, error: %d", err);
        return -1;
    }
    dev->read_page_delay_us = 25;
    dev->erase_block_delay_us = 3200;
    dev->program_page_delay_us = 380;
    switch (device_id) {
    case GIGADEVICE_DI_51:
    case GIGADEVICE_DI_41:
    case GIGADEVICE_DI_31:
    case GIGADEVICE_DI_21:
        dev->dhara_nand.num_blocks = 1024;
        break;
    case GIGADEVICE_DI_52:
    case GIGADEVICE_DI_42:
    case GIGADEVICE_DI_32:
    case GIGADEVICE_DI_22:
        dev->dhara_nand.num_blocks = 2048;
        break;
    case GIGADEVICE_DI_55:
    case GIGADEVICE_DI_45:
    case GIGADEVICE_DI_35:
    case GIGADEVICE_DI_25:
        dev->dhara_nand.num_blocks = 4096;
        break;
    default:
        LOG_ERR("Invalid Gigadevice device ID: %u", device_id);
        return -1;
    }
    return 0;
}


/**
 * @brief Detects the NAND flash chip and initializes it based on the manufacturer ID.
 *
 * This function reads the manufacturer ID of the NAND flash chip and calls the appropriate
 * initialization function based on the detected manufacturer. It supports chips from
 * Alliance, Winbond, and GigaDevice. The function sends a command to read the manufacturer
 * ID and then, based on the ID, calls a specific initialization function to set up device
 * parameters and operational characteristics.
 *
 * @param dev Pointer to the spi_nand_flash_device_t structure representing the SPI NAND device.
 * @return 0 on successful detection and initialization of the chip, -1 on failure with an error logged.
 */
static int detect_chip(spi_nand_flash_device_t *dev)
{
    uint8_t manufacturer_id;
    spi_nand_transaction_t t = {
        .command = CMD_READ_ID,
        .address = 0, // This normally selects the manufacturer id. Some chips ignores it, but still expects 8 dummy bits here
        .address_bytes = 1,
        .miso_len = 1,
        .miso_data = &manufacturer_id
    };
    spi_nand_execute_transaction(dev->config.spi_dev, &t);

    switch (manufacturer_id) {
    case SPI_NAND_FLASH_ALLIANCE_MI: // Alliance
        return spi_nand_alliance_init(dev);
    case SPI_NAND_FLASH_WINBOND_MI: // Winbond
        return spi_nand_winbond_init(dev);
    case SPI_NAND_FLASH_GIGADEVICE_MI: // GigaDevice
        return spi_nand_gigadevice_init(dev);
    default:
        LOG_ERR("Invalid manufacturer ID: %u", manufacturer_id);
        return -1;
    }
}

/**
 * @brief Unprotects the NAND flash chip to enable write operations.
 *
 * This function checks if the chip is currently protected by reading the protection
 * register (REG_PROTECT). If the chip is protected, it sends a command to write to the
 * protection register and clear the protection bits, thereby unprotecting the chip.
 * This allows subsequent operations such as erase and program to proceed without being
 * blocked by write protection.
 *
 * @param dev Pointer to the spi_nand_flash_device_t structure representing the SPI NAND device.
 * @return 0 on successful unprotection of the chip, -1 on failure.
 */
static int unprotect_chip(spi_nand_flash_device_t *dev)
{
    uint8_t status;
    int ret = spi_nand_read_register(dev->config.spi_dev, REG_PROTECT, &status);
    if (ret != 0) {
        LOG_ERR("Failed to read register: %d", err);
        return ret;
    }

    if (status != 0x00) {
        ret = spi_nand_write_register(dev->config.spi_dev, REG_PROTECT, 0);
    }
    if (ret != 0) {
        LOG_ERR("Failed to remove protection bit with error code: %d", err);
        return -1;
    }

    return 0;
}



//////////////////////////          END STATIC FUNCTIONS            /////////////////////////////////////



int wait_for_ready(const struct spi_dt_spec *device, uint32_t expected_operation_time_us, uint8_t *status_out)
{
    // Assuming ROM_WAIT_THRESHOLD_US is defined somewhere globally
    if (expected_operation_time_us < ROM_WAIT_THRESHOLD_US) {
        k_busy_wait(expected_operation_time_us); // busy wait for microseconds
    }

    while (true) {
        uint8_t status;
        int err = spi_nand_read_register(device, REG_STATUS, &status);
        if (err != 0) {
            LOG_ERR("Error reading NAND status register");
            return -1; 
        }

        if ((status & STAT_BUSY) == 0) {
            if (status_out) {
                *status_out = status;
            }
            break;
        }

        if (expected_operation_time_us >= ROM_WAIT_THRESHOLD_US) {
            k_sleep(K_MSEC(1)); // Sleep for 1 millisecond instead of using vTaskDelay
        }
    }

    return 0; 
}


int spi_nand_flash_init_device(spi_nand_flash_config_t *config, spi_nand_flash_device_t **handle)
{
    // Verify SPI device is ready
    if (!device_is_ready(config->spi_dev)) {
        LOG_ERR("SPI device is not ready");
        return -1;
    }

    // Apply default garbage collection factor if not set
    if (config->gc_factor == 0) {
        config->gc_factor = 45;
    }

    // Allocate memory for the NAND flash device structure
    *handle = calloc(1, sizeof(spi_nand_flash_device_t));
    if (*handle == NULL) {
        LOG_ERR("Failed to allocate memory for NAND flash device");
        return -1;
    }

    // Copy the configuration into the device structure
    memcpy(&(*handle)->config, config, sizeof(spi_nand_flash_config_t));

    (*handle)->dhara_nand.log2_ppb = 6; // Assume 64 pages per block
    (*handle)->dhara_nand.log2_page_size = 11; // Assume 2048 bytes per page

    int ret = detect_chip(*handle);
    if (ret != 0) {
        LOG_ERR("Failed to detect NAND chip");
        goto fail;
    }

    // Unprotect the NAND flash chip
    ret = unprotect_chip(*handle);
    if (ret != 0) {
        LOG_ERR("Failed to unprotect NAND chip");
        goto fail;
    }

    // Calculate size parameters based on detected chip
    (*handle)->page_size = 1 << (*handle)->dhara_nand.log2_page_size;
    (*handle)->block_size = (1 << (*handle)->dhara_nand.log2_ppb) * (*handle)->page_size;
    (*handle)->num_blocks = (*handle)->dhara_nand.num_blocks;

    // Allocate work buffer for NAND operations
    (*handle)->work_buffer = malloc((*handle)->page_size);
    if ((*handle)->work_buffer == NULL) {
        LOG_ERR("Failed to allocate work buffer");
        ret = -1;
        goto fail;
    }

    // Initialize mutex for thread safety
    k_mutex_init(&(*handle)->mutex);

    dhara_map_init(&(*handle)->dhara_map, &(*handle)->dhara_nand, (*handle)->work_buffer, config->gc_factor);
    

    // Resume map to handle power failures
    dhara_error_t ignored;
    dhara_map_resume(&(*handle)->dhara_map, &ignored);

    return 0;

fail:
    if ((*handle)->work_buffer != NULL) {
        free((*handle)->work_buffer);
    }
    free(*handle);
    return ret;
}



int spi_nand_erase_chip(spi_nand_flash_device_t *handle)
{
    LOG_WRN("Entire chip is being erased");
    int ret;

    // Take the semaphore with K_FOREVER to wait indefinitely
    k_sem_take(&handle->mutex, K_FOREVER);

    for (int i = 0; i < handle->num_blocks; i++) {
        ret = spi_nand_write_enable(handle->config.spi_dev);
        if (ret != 0) {
            LOG_ERR("Failed to enable write for block erase");
            goto end;
        }

        ret = spi_nand_erase_block(handle->config.spi_dev, i * (1 << handle->dhara_nand.log2_ppb));
        if (ret != 0) {
            LOG_ERR("Failed to erase block");
            goto end;
        }

        ret = wait_for_ready(handle->config.spi_dev, handle->erase_block_delay_us, NULL);
        if (ret != 0) {
            LOG_ERR("Failed to wait for readiness after erase");
            goto end;
        }
    }

    // clear dhara map
    dhara_map_init(&handle->dhara_map, &handle->dhara_nand, handle->work_buffer, handle->config.gc_factor);
    dhara_map_clear(&handle->dhara_map);

    // Operation succeeded, release the semaphore and return success
    k_sem_give(&handle->mutex);
    return 0;

end:
    // Release the semaphore in case of error as well
    k_sem_give(&handle->mutex);
    return ret;
}


int spi_nand_flash_read_sector(spi_nand_flash_device_t *handle, uint8_t *buffer, uint16_t sector_id)
{
    dhara_error_t err;
    int ret = 0;

    k_sem_take(&handle->mutex, K_FOREVER);

    if (dhara_map_read(&handle->dhara_map, sector_id, buffer, &err)) {
        // Map your error codes appropriately
        ret = -1;
    } else if (err) {
        // This indicates a soft ECC error, we rewrite the sector to recover
        if (dhara_map_write(&handle->dhara_map, sector_id, buffer, &err)) {
            // Map your error codes appropriately
            ret = -1; // Example error code, adjust based on your error handling
        }
    }

    // Use Zephyr's semaphore give function
    k_sem_give(&handle->mutex);
    return ret;
}


int spi_nand_flash_write_sector(spi_nand_flash_device_t *handle, const uint8_t *buffer, uint16_t sector_id)
{
    dhara_error_t err;
    int ret = 0; 

    k_sem_take(&handle->mutex, K_FOREVER);

    if (dhara_map_write(&handle->dhara_map, sector_id, buffer, &err)) {
        
        ret = -1; // Example error code, adjust based on your error handling
    }

    // Use Zephyr's semaphore give function
    k_sem_give(&handle->mutex);
    return ret;
}


int spi_nand_flash_sync(spi_nand_flash_device_t *handle)
{
    dhara_error_t err;
    int ret = 0;

    k_sem_take(&handle->mutex, K_FOREVER);

    if (dhara_map_sync(&handle->dhara_map, &err)) {
        ret = -1; 
    }

    k_sem_give(&handle->mutex);
    return ret;
}


int spi_nand_flash_get_capacity(spi_nand_flash_device_t *handle, uint16_t *number_of_sectors)
{
    *number_of_sectors = dhara_map_capacity(&handle->dhara_map);
    return 0; 
}


int spi_nand_flash_get_sector_size(spi_nand_flash_device_t *handle, uint16_t *sector_size)
{
    *sector_size = handle->page_size;
    return 0;
}

int spi_nand_flash_deinit_device(spi_nand_flash_device_t *handle)
{
    free(handle->work_buffer);
    free(handle);
    return 0;
}
