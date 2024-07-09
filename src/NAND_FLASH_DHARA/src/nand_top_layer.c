#include <stdlib.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>


#include "../inc/nand_oper.h"
#include "../dhara/dhara/nand.h"
#include "../inc/nand_top_layer.h"
#include "../inc/nand_flash_devices.h"
#include "../inc/example_handle.h"

LOG_MODULE_REGISTER(nand_top_layer, CONFIG_LOG_DEFAULT_LEVEL);


//shared externally

static nand_flash_device_t nand_flash_device;
nand_flash_device_t *device_handle = &nand_flash_device;


/**
 * @brief Initialize a Winbond NAND device.
 *
 * This function initializes a Winbond NAND flash device by reading its
 * device ID and setting up specific timing parameters based on the device ID.
 * *
 * @param dev Pointer to the NAND device structure.
 *
 * @retval 0 If the initialization is successful.
 * @retval -1 If there was an error during device ID retrieval or if the device ID
 *         is not recognized as a supported Winbond device.
 */
static int nand_winbond_init(nand_flash_device_t *dev)
{
    uint8_t device_id_buf[2];
    nand_transaction_t t = {
        .command = CMD_READ_ID,
        .dummy_bytes = 2,
        .miso_len = 2,
        .miso_data = device_id_buf
    };
    int err = my_nand_handle->transceive(&t);
    if (err != 0) {
        LOG_ERR("Failed to receive device ID, error: %d", err);
        return -1;
    }

    uint16_t device_id = (device_id_buf[0] << 8) + device_id_buf[1];
    dev->dhara_nand.log2_ppb = 6; // Assume 64 pages per block
    dev->dhara_nand.log2_page_size = 11;// Assume 2048 bytes per page

    switch (device_id) {
    case WINBOND_DI_AA20:
        LOG_INF("Automatic recognition of WINBOND_DI_AA20 flash");
    case WINBOND_DI_BA20:
        LOG_INF("Automatic recognition of WINBOND_DI_BA20 flash");
        dev->dhara_nand.num_blocks = 512;
        break;
    case WINBOND_DI_AA21:
        LOG_INF("Automatic recognition of WINBOND_DI_AA21 flash");
    case WINBOND_DI_BA21:
        LOG_INF("Automatic recognition of WINBOND_DI_BA21 flash");
    case WINBOND_DI_BC21:
        LOG_INF("Automatic recognition of WINBOND_DI_BC21 flash");
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
 * This function reads the device ID using a transaction, then sets
 * device-specific timing parameters and the number of blocks based on the
 * device ID. It supports multiple Alliance Memory NAND flash devices by
 * adjusting configuration parameters accordingly.
 *
 * @param dev Pointer to the NAND flash device structure.
 * @return 0 on success, -1 on error with a logged error message.
 */

static int nand_alliance_init(nand_flash_device_t *dev)
{
    int err;
    uint8_t device_id;
    nand_transaction_t t = {
        .command = CMD_READ_ID,
        .address =  DEVICE_ADDR_READ,
        .address_bytes = 1,
        .miso_len = 1,
        .miso_data = &device_id
    };
    err = my_nand_handle->transceive(&t);
    if (err != 0) {
        LOG_ERR("Failed to read Alliance device ID, error: %d", err);
        return -1;
    }
    //setting up the device

    dev->dhara_nand.log2_ppb = 6; // Assume 64 pages per block
    dev->dhara_nand.log2_page_size = 11;// Assume 2048 bytes per page
    switch (device_id) {
    case ALLIANCE_DI_25: //AS5F31G04SND-08LIN
        LOG_INF("Automatic recognition of AS5F31G04SND-08LIN flash");
        dev->dhara_nand.num_blocks = 1024;
        break;
    case ALLIANCE_DI_2E: //AS5F32G04SND-08LIN
        LOG_INF("Automatic recognition of AS5F32G04SND-08LIN flash");
    case ALLIANCE_DI_8E: //AS5F12G04SND-10LIN
        LOG_INF("Automatic recognition of AS5F12G04SND-10LIN flash");
        dev->dhara_nand.num_blocks = 2048;
        break;
    case ALLIANCE_DI_2F: //AS5F34G04SND-08LIN
        LOG_INF("Automatic recognition of AS5F34G04SND-08LIN flash");
    case ALLIANCE_DI_8F: //AS5F14G04SND-10LIN ==> Current implementation
        LOG_INF("NAND MAPPING LAYER: Automatic recognition of AS5F14G04SND-10LIN flash");
        dev->dhara_nand.num_blocks = 4096;
        break;
    case ALLIANCE_DI_2D: //AS5F38G04SND-08LIN
        LOG_INF("Automatic recognition of AS5F38G04SND-08LIN flash");
    case ALLIANCE_DI_8D: //AS5F18G04SND-10LIN
        LOG_INF("Automatic recognition of AS5F18G04SND-10LIN flash");
        dev->dhara_nand.log2_page_size = 12; // 4k pages
        dev->dhara_nand.num_blocks = 4096;
        break;
    default:
        LOG_ERR("Invalid Alliance device ID: 0x%X", device_id);
            return -1;
    }
    return 0;
}

/**
 * @brief Initializes a GigaDevice NAND flash based on its device ID.
 *
 * This function reads the device ID of a GigaDevice NAND flash and configures
 * various operational parameters based on the detected device.
 *
 * @param dev Pointer to the nand_flash_device_t structure representing the NAND device.
 * @return 0 on successful initialization and configuration, -1 on failure with an error logged.
 */
static int nand_gigadevice_init(nand_flash_device_t *dev)
{
    uint8_t device_id;
    int err;
    nand_transaction_t t = {
        .command = CMD_READ_ID,
        .dummy_bytes = 2,
        .miso_len = 1,
        .miso_data = &device_id
    };
    err = my_nand_handle->transceive(&t);
    if (err != 0) {
        LOG_ERR("Failed to read gigadevice device ID, error: %d", err);
        return -1;
    }
    dev->dhara_nand.log2_ppb = 6; // Assume 64 pages per block
    dev->dhara_nand.log2_page_size = 11;// Assume 2048 bytes per page
    switch (device_id) {
    case GIGADEVICE_DI_51:
        LOG_INF("Automatic recognition of GIGADEVICE_DI_51 flash");
    case GIGADEVICE_DI_41:
        LOG_INF("Automatic recognition of GIGADEVICE_DI_41 flash");
    case GIGADEVICE_DI_31:
        LOG_INF("Automatic recognition of GIGADEVICE_DI_31 flash");
    case GIGADEVICE_DI_21:
        LOG_INF("Automatic recognition of GIGADEVICE_DI_21 flash");
        dev->dhara_nand.num_blocks = 1024;
        break;
    case GIGADEVICE_DI_52:
        LOG_INF("Automatic recognition of GIGADEVICE_DI_52 flash");
    case GIGADEVICE_DI_42:
        LOG_INF("Automatic recognition of GIGADEVICE_DI_42 flash");
    case GIGADEVICE_DI_32:
        LOG_INF("Automatic recognition of GIGADEVICE_DI_32 flash");
    case GIGADEVICE_DI_22:
        LOG_INF("Automatic recognition of GIGADEVICE_DI_22 flash");
        dev->dhara_nand.num_blocks = 2048;
        break;
    case GIGADEVICE_DI_55:
        LOG_INF("Automatic recognition of GIGADEVICE_DI_55 flash");
    case GIGADEVICE_DI_45:
        LOG_INF("Automatic recognition of GIGADEVICE_DI_45 flash");
    case GIGADEVICE_DI_35:
        LOG_INF("Automatic recognition of GIGADEVICE_DI_35 flash");
    case GIGADEVICE_DI_25:
        LOG_INF("Automatic recognition of GIGADEVICE_DI_25 flash");
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
 * @param dev Pointer to the nand_flash_device_t structure representing the NAND device.
 * @return 0 on successful detection and initialization of the chip, -1 on failure with an error logged.
 */
static int detect_chip(nand_flash_device_t *dev)
{
    uint8_t manufacturer_id;
    nand_transaction_t t = {
        .command = CMD_READ_ID,
        .address = 0, // This normally selects the manufacturer id. Some chips ignores it, but still expects 8 dummy bits here
        .address_bytes = 1,
        .miso_len = 1,
        .miso_data = &manufacturer_id
    };
    my_nand_handle->transceive(&t);

    dev->gc_factor = 12;//after investigation this factor is the most fitting for the motion tracker

    switch (manufacturer_id) {
    case NAND_FLASH_ALLIANCE_MI: // Alliance
        return nand_alliance_init(dev);
    case NAND_FLASH_WINBOND_MI: // Winbond
        return nand_winbond_init(dev);
    case NAND_FLASH_GIGADEVICE_MI: // GigaDevice
        return nand_gigadevice_init(dev);
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
 * @param dev Pointer to the nand_flash_device_t structure representing the NAND device.
 * @return 0 on successful unprotection of the chip, -1 on failure.
 */
static int unprotect_chip(nand_flash_device_t *dev)
{
    uint8_t status;
    int ret = nand_read_register(REG_PROTECT, &status);
    if (ret != 0) {
        LOG_ERR("Failed to read register: %d", ret);
        return ret;
    }

    if (status != 0x00) {
        ret = nand_write_register(REG_PROTECT, 0);
    }
    if (ret != 0) {
        LOG_ERR("Failed to remove protection bit with error code: %d", ret);
        return -1;
    }

    return 0;
}



//////////////////////////          END STATIC FUNCTIONS            /////////////////////////////////////



int wait_for_ready(uint8_t *status_out)
{
    while (true) {
        uint8_t status;
        int err = nand_read_register(REG_STATUS, &status);
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
    }

    return 0; 
}




int nand_flash_init_device(nand_flash_device_t **handle)
{
    LOG_INF("NAND MAPPING LAYER: Initializing DHARA mapping");
        
    *handle = device_handle;

    // Apply default garbage collection factor if not set
    if ((*handle)->gc_factor == 0) {
        (*handle)->gc_factor = 15;
    }


    if (*handle == NULL) {
        LOG_ERR("Failed to allocate memory for NAND flash device");
        return -1;
    }

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
    //(*handle)->work_buffer = malloc((*handle)->page_size);
    if ((*handle)->work_buffer == NULL) {
        (*handle)->work_buffer = malloc((*handle)->page_size);
    }


    if ((*handle)->work_buffer == NULL) {
        LOG_ERR("Failed to allocate work buffer");
        ret = -1;
        goto fail;
    }

    // Initialize mutex for thread safety
    // Initialize the semaphore with an initial count of 1 and a maximum count of 1
    // This means the semaphore is immediately available for one `take` operation (semaphore signals not locks)
    k_sem_init(&(*handle)->mutex, 1, 1);
    

    dhara_map_init(&(*handle)->dhara_map, &(*handle)->dhara_nand, (*handle)->work_buffer, (*handle)->gc_factor);
    

    // Resume map to handle power failures
    dhara_error_t ignored;
    ret = dhara_map_resume(&(*handle)->dhara_map, &ignored);
    if (ret == -1) {
        LOG_INF("No valid stored state, reinitializing map");
    }

    return 0;

fail:
    if ((*handle)->work_buffer != NULL) {
        free((*handle)->work_buffer);
    }
    LOG_ERR("Failed to initalize mapping");
    return ret;
}



int nand_erase_chip(nand_flash_device_t *handle)
{
    LOG_WRN("Entire chip is being erased");
    int ret;

    // Take the semaphore with K_FOREVER to wait indefinitely
    k_sem_take(&handle->mutex, K_FOREVER);

    for (int i = 0; i < handle->num_blocks; i++) {
        ret = nand_write_enable();
        if (ret != 0) {
            LOG_ERR("Failed to enable write for block erase");
            goto end;
        }

        ret = nand_erase_block(i * (1 << handle->dhara_nand.log2_ppb));
        if (ret != 0) {
            LOG_ERR("Failed to erase block");
            goto end;
        }


        ret = wait_for_ready(NULL);
        if (ret != 0) {
            LOG_ERR("Failed to wait for readiness after erase");
            goto end;
        }
    }

    // clear dhara map
    dhara_map_init(&handle->dhara_map, &handle->dhara_nand, handle->work_buffer, handle->gc_factor);
    dhara_map_clear(&handle->dhara_map);

    k_sem_give(&handle->mutex);
    return 0;

end:
    // Release the semaphore in case of error as well
    k_sem_give(&handle->mutex);
    return ret;
}


int nand_flash_read_sector(nand_flash_device_t *handle, uint8_t *buffer, uint16_t sector_id)
{
    dhara_error_t err;
    int ret = 0;

    k_sem_take(&handle->mutex, K_FOREVER);

    if (dhara_map_read(&handle->dhara_map, sector_id, buffer, &err) == 0) {
        ret = err;
    } else if (err == DHARA_E_ECC) {
        // This indicates a soft ECC error, we rewrite the sector to recover
        LOG_INF("Soft ECC error, recovering");
        if (dhara_map_write(&handle->dhara_map, sector_id, buffer, &err)) {
            ret = err;
            LOG_ERR("error while writing to map"); 
        }
    }
    k_sem_give(&handle->mutex);
    return ret;
}


int nand_flash_write_sector(nand_flash_device_t *handle, const uint8_t *buffer, uint16_t sector_id)
{
    dhara_error_t err;
    int ret = 0; 

    k_sem_take(&handle->mutex, K_FOREVER);

    if (dhara_map_write(&handle->dhara_map, sector_id, buffer, &err)) {
        LOG_ERR("error while writing to map"); 
        ret = err; 
    }

    k_sem_give(&handle->mutex);
    return ret;
}


int nand_flash_sync(nand_flash_device_t *handle)
{
    dhara_error_t err;
    int ret = 0;

    k_sem_take(&handle->mutex, K_FOREVER);

    if (dhara_map_sync(&handle->dhara_map, &err)) {
        ret = err; 
    }

    k_sem_give(&handle->mutex);
    return ret;
}


int nand_flash_get_capacity(nand_flash_device_t *handle, uint32_t *number_of_sectors)
{
    *number_of_sectors = dhara_map_capacity(&handle->dhara_map);
    return 0; 
}


int nand_flash_get_sector_size(nand_flash_device_t *handle, uint32_t *sector_size)
{
    *sector_size = handle->page_size;
    return 0;
}

int nand_flash_deinit_device(nand_flash_device_t *handle)
{
    if (handle->work_buffer != NULL) {
        free(handle->work_buffer);
        handle->work_buffer = NULL;
    }
    //free(handle);
    return 0;
}
