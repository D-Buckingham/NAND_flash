//This file has to be renamed and go into the dhara folder
//rename to nand.c

/**
 * @file nand.c
 * @brief Conection between the spi communication layer and the DHARA FTL
 *
 * This file provides the missing implementations that are zephyr framework specific to
 * the dhara flash translation layer (FTL)
 * Author: [Denis Buckingham]
 * Date: [15.03.2024]
 */

#include "nand.h"
#include "spi_nand_oper.h"
#include "nand_top_layer.h"//for the spi_nand_flash_device_t


#include <zephyr/drivers/spi.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "map.h"


#include <string.h>
#include <stdlib.h>

LOG_MODULE_REGISTER(dhara_glue, CONFIG_LOG_DEFAULT_LEVEL);

#define ROM_WAIT_THRESHOLD_US 1000

//defined in 

/** @brief waiting for finished transaction
 * wait at least the expected time and then check for the status register
 *
 * defined in the nand.c between spi_nand_oper and dhara
 * used in the top layer and nand.c
 *
 * @param device Device SPI configuration data obtained from devicetree.
 * @param expected_operation_time_us
 * @param[out] status_out status register content of current transaction
 * @return 0 on success, -1 if the read out of the register failed.
 */
static int wait_for_ready_nand(const struct spi_dt_spec *device, uint32_t expected_operation_time_us, uint8_t *status_out)
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
            k_sleep(K_MSEC(1)); 
        }
    }

    return 0; // Success
}



/**
 * @brief Read a NAND flash page and wait for the operation to complete.
 * 
 * This function initiates a read operation for a specified page in the NAND flash
 * and waits until the device is ready or until an error occurs. It leverages the
 * `spi_nand_read_page` function to perform the read operation and checks the device's
 * status by calling `wait_for_ready_nand`.
 * 
 * @param device A pointer to the spi_nand_flash_device_t structure representing the NAND flash device.
 * @param page The page number to read from the NAND flash.
 * @param status_out Pointer to a variable where the status register's value will be stored upon successful read. 
 *                   Can be NULL if the status is not needed.
 * 
 * @return 0 on success, -1 on failure.
 */
static int read_page_and_wait(struct spi_nand_flash_device_t *device, uint32_t page, uint8_t *status_out)
{
    int err;
    err = spi_nand_read_page(device -> config.spi_dev, page); 
    if (err != 0) {
        LOG_ERR("Failed to read page %u, error: %d", page, err);
        return -1;
    }

    return wait_for_ready_nand(device -> config.spi_dev,device -> read_page_delay_us, status_out);
}


/**
 * @brief Program a NAND flash page and wait for the operation to complete.
 * 
 * This function initiates a program operation for a specified page in the NAND flash
 * and waits until the device is ready or until an error occurs. It leverages the
 * `spi_nand_program_execute` function to perform the program operation and checks the device's
 * status by calling `wait_for_ready_nand`.
 * 
 * The function ensures that the programming operation is executed correctly by monitoring
 * the NAND flash device's status register after the operation. It makes use of the delay
 * specified in `device->program_page_delay_us` to determine the wait period for the device
 * to become ready.
 * 
 * @param device A pointer to the spi_nand_flash_device_t structure representing the NAND flash device.
 * @param page The page number where the data will be programmed in the NAND flash.
 * @param status_out Pointer to a variable where the status register's value will be stored upon successful programming. 
 *                   Can be NULL if the status is not needed.
 * 
 * @return 0 on success, -1 on failure.
 */
static int program_execute_and_wait(struct spi_nand_flash_device_t *device, uint32_t page, uint8_t *status_out)
{
    int err;

    err = spi_nand_program_execute(device -> config.spi_dev, page);
    if (err != 0) {
        LOG_ERR("Failed to execute program on page %u, error: %d", page, err);
        return -1;
    }

    return wait_for_ready_nand(device -> config.spi_dev, device -> program_page_delay_us, status_out);
}

#define SPARE_AREA_OFFSET_1 0x816
#define SPARE_AREA_OFFSET_2 0x820
//write to first page in block spare area 816h 820h how many times it was erased
static int erase_counter_increased(dhara_page_t first_block_page, struct spi_nand_flash_device_t *device) {
    uint32_t erase_count_indicator = 0;
    int ret;

    // Read the first page of the block
    ret = read_page_and_wait(device, first_block_page, NULL);
    if (ret != 0) {
        LOG_ERR("Error reading page %u: %d", first_block_page, ret);
        return ret;
    }

    // Read the current erase count indicator from the spare area
    ret = spi_nand_read(device->config.spi_dev, (uint8_t *)&erase_count_indicator, SPARE_AREA_OFFSET_1, 4);
    if (ret != 0) {
        LOG_ERR("Failed to read erase count from spare area: %d", ret);
        return ret;
    }

    LOG_INF("Current erase count for block at page %u: %u", first_block_page, erase_count_indicator);

    // Increment the erase count
    erase_count_indicator++;

    // Enable writing to the NAND device
    ret = spi_nand_write_enable(device->config.spi_dev);
    if (ret != 0) {
        LOG_ERR("Failed to enable write: %d", ret);
        return ret;
    }

    // Load the incremented erase count into the NAND device's cache
    ret = spi_nand_program_load(device->config.spi_dev, (uint8_t *)&erase_count_indicator, SPARE_AREA_OFFSET_1, 4);
    if (ret != 0) {
        LOG_ERR("Failed to load program with new erase count: %d", ret);
        return ret;
    }

    // Execute the program operation to write the new erase count to the NAND device
    ret = program_execute_and_wait(device, first_block_page, NULL);
    if (ret != 0) {
        LOG_ERR("Failed to execute program and wait: %d", ret);
        return ret;
    }

    LOG_INF("Updated erase count for block at page %u to %u", first_block_page, erase_count_indicator);

    return 0; // Success
}


/**
 * @return 1 if block is bad, 0 (false) if the block is good (indicator equals 0xFFFF),
*/
int dhara_nand_is_bad(const struct dhara_nand *n, dhara_block_t b)
{
    
    /**
     * to retrieve a pointer to a container structure given a pointer to a member of that structure. It's used to obtain
     * a pointer to a parent or enclosing structure by knowing the pointer to a member field and the type of the parent 
     * structure.
    */
    spi_nand_flash_device_t *dev = CONTAINER_OF(n, spi_nand_flash_device_t, dhara_nand);//struct necessary?

    dhara_page_t first_block_page = b * (1 << n->log2_ppb);
    uint16_t bad_block_indicator;
    int ret;

    ret = read_page_and_wait(dev, first_block_page, NULL);
    if (ret != 0) {
        LOG_ERR("Error reading page: %d", ret);
        return 1; // Assume bad block on error
    }

    // Assuming spi_nand_read function reads 'len' bytes starting from 'offset' to 'buf'
    // And assuming the OOB data can be accessed directly following the main data area
    ret = spi_nand_read(dev->config.spi_dev, (uint8_t *)&bad_block_indicator, dev->page_size, 2);
    if (ret != 0) {
        LOG_ERR("Failed to read bad block indicator, err: %d", ret);
        return 1; // Assume bad block on error
    }

    LOG_DBG("Block=%u, Page=%u, Indicator=%04x", b, first_block_page, bad_block_indicator);
    if(bad_block_indicator == 0x0000){LOG_INF("Bad_Block on Block=%u, Page=%u", b, first_block_page);}
    return bad_block_indicator == 0x0000;//!= 0xFFFF;//changed because of accidental changes
}



//TODO check in datasheet process
void dhara_nand_mark_bad(const struct dhara_nand *n, dhara_block_t b)
{
    spi_nand_flash_device_t *dev = CONTAINER_OF(n, spi_nand_flash_device_t, dhara_nand);//struct?
    int ret;

    dhara_page_t first_block_page = b * (1 << n->log2_ppb);
    uint16_t bad_block_indicator = 0;
    LOG_DBG("mark_bad, block=%u, page=%u, indicator = %04x", b, first_block_page, bad_block_indicator);

    ret = spi_nand_write_enable(dev->config.spi_dev);
    if (ret) {
        LOG_ERR("Failed to enable write, error: %d", ret);
        return;
    }

    ret = spi_nand_erase_block(dev->config.spi_dev, first_block_page);
    if (ret != 0) {
        LOG_ERR("Failed to erase block, error: %d", ret);
        return;
    }

    ret = spi_nand_write_enable(dev->config.spi_dev);
    if (ret != 0) {
        LOG_ERR("Failed to enable write, error: %d", ret);
        return;
    }

    ret = spi_nand_program_load(dev->config.spi_dev, (uint8_t *)&bad_block_indicator, dev->page_size, 2);
    if (ret != 0) {
        LOG_ERR("Failed to program load, error: %d", ret);
        return;
    }

    ret = program_execute_and_wait(dev, first_block_page, NULL);
    if (ret != 0) {
        LOG_ERR("Failed to execute program and wait, error: %d", ret);
        return;
    }
}


/* Erase the given block. This function should return 0 on success or -1
 * on failure.
 *
 * The status reported by the chip should be checked. If an erase
 * operation fails, return -1 and set err to E_BAD_BLOCK.
 */
int dhara_nand_erase(const struct dhara_nand *n, dhara_block_t b, dhara_error_t *err)
{
    LOG_INF("erase_block, block=%u", b);
    struct spi_nand_flash_device_t *dev = CONTAINER_OF(n, struct spi_nand_flash_device_t, dhara_nand);
    int ret;

    dhara_page_t first_block_page = b * (1 << n->log2_ppb);
    uint8_t status;

    ret = spi_nand_write_enable(dev->config.spi_dev);
    if (ret != 0) {
        LOG_ERR("Failed to enable write, error: %d", ret);
        return -1;
    }

    ret = spi_nand_erase_block(dev->config.spi_dev, first_block_page);
    if (ret != 0) {
        LOG_ERR("Failed to erase block, error: %d", ret);
        return -1;
    }

    ret = wait_for_ready_nand(dev->config.spi_dev, dev->erase_block_delay_us, &status);
    if (ret != 0) {
        LOG_ERR("Failed to wait for ready, error: %d", ret);
        return -1;
    }

    //write to first page in block spare area 816h 820h how many times it was erased
    ret = erase_counter_increased(first_block_page, dev);
    if(ret!= 0){
        LOG_ERR("Failed to increase the erase counter");
    }
   

    if ((status & STAT_ERASE_FAILED) != 0) {
        dhara_set_error(err, DHARA_E_BAD_BLOCK);
        LOG_ERR("Erasing failed, indicated by status register");
        return -1;
    }

    return 0;
}


/* Program the given page. The data pointer is a pointer to an entire
 * page ((1 << log2_page_size) bytes). The operation status should be
 * checked. If the operation fails, return -1 and set err to
 * E_BAD_BLOCK.
 * 
 */
int dhara_nand_prog(const struct dhara_nand *n, dhara_page_t p, const uint8_t *data, dhara_error_t *err)
{
    LOG_DBG("prog, page=%u", p);
    spi_nand_flash_device_t *dev = CONTAINER_OF(n, spi_nand_flash_device_t, dhara_nand);
    int ret;
    uint8_t status;
    uint16_t used_marker = 0;

    ret = read_page_and_wait(dev, p, NULL);//Initiate page read operation to cache. Reads the specified page from NAND to the device's internal cache.
    if (ret) {
        LOG_ERR("Failed to read page %u", p);
        return -1;
    }

    ret = spi_nand_write_enable(dev->config.spi_dev);//Enable writing on the SPI NAND device.
    if (ret) {
        LOG_ERR("Failed to enable write, error: %d", ret);
        return -1;
    }


    ret = spi_nand_program_load(dev->config.spi_dev, data, 0, dev->page_size);//Load data into the SPI NAND device's cache.
    if (ret) {
        LOG_ERR("Failed to load program, error: %d", ret);
        return -1;
    }
    // Log the data written, 40 bytes per line ==> lead to entire buffer filled, go deeper
    // LOG_INF("Data write to page  %u in nand.c:", p);
    // for (size_t i = 0; i < dev->page_size; i++) {
    //     if (i % 40 == 0 && i != 0) {
    //         LOG_INF("");  // New line every 40 bytes, but not at the start
    //     }
    //     printk("%02X ", data[i]);  // Using printk for continuous output on the same line
    // }
    // if (dev->page_size % 40 != 0) {
    //     LOG_INF("");  // Ensure ending on a new line if not already done
    // }

    ret = spi_nand_program_load(dev->config.spi_dev, (uint8_t *)&used_marker, dev->page_size + 2, 2);//put a flag there
    if (ret) {
        LOG_ERR("Failed to load used marker, error: %d", ret);
        return -1;
    }

    ret = program_execute_and_wait(dev, p, &status);//Execute a program operation. Commits the data previously loaded into the device's cache to the NAND array
    if (ret) {
        LOG_ERR("Failed to execute program, error: %d", ret);
        return -1;
    }

    if ((status & STAT_PROGRAM_FAILED) != 0) {
        LOG_INF("prog failed, page=%u", p);
        dhara_set_error(err, DHARA_E_BAD_BLOCK);
        return -1;
    }

    return 0;
}




int dhara_nand_is_free(const struct dhara_nand *n, dhara_page_t p)
{
    spi_nand_flash_device_t *dev = CONTAINER_OF(n, spi_nand_flash_device_t, dhara_nand);
    int ret;
    uint16_t used_marker = 0;

    ret = read_page_and_wait(dev, p, NULL);
    if (ret) {
        LOG_ERR("Failed to read page %u", p);
        return 0; 
    }

    ret = spi_nand_read(dev->config.spi_dev, (uint8_t *)&used_marker, dev->page_size + 2, 2);
    if (ret) {
        LOG_ERR("Failed to read OOB area for page %u", p);
        return 0; 
    }

    LOG_DBG("Is free, page=%u, used_marker=%04x", p, used_marker);
    return used_marker == 0xFFFF; // Check against expected marker value for a free page
}

static int increase_ECC_counter(struct spi_nand_flash_device_t *device, uint32_t page){

}

#define ECC_SPARE_AREA_OFFSET_1 0x821
#define ECC_SPARE_AREA_OFFSET_2 0x824
static int increase_ECC_counter(struct spi_nand_flash_device_t *device, uint32_t page) {
    uint32_t ecc_count_indicator = 0;
    int ret;

    ret = read_page_and_wait(device, page, NULL);
    if (ret != 0) {
        LOG_ERR("Error reading page %u: %d", page, ret);
        return ret;
    }

    ret = spi_nand_read(device->config.spi_dev, (uint8_t *)&ecc_count_indicator, ECC_SPARE_AREA_OFFSET_1, 4);
    if (ret != 0) {
        LOG_ERR("Failed to read ECC count from spare area: %d", ret);
        return ret;
    }

    LOG_INF("Current ECC count for page %u: %u", page, ecc_count_indicator);

    ecc_count_indicator++;

    ret = spi_nand_write_enable(device->config.spi_dev);
    if (ret != 0) {
        LOG_ERR("Failed to enable write: %d", ret);
        return ret;
    }

    ret = spi_nand_program_load(device->config.spi_dev, (uint8_t *)&ecc_count_indicator, ECC_SPARE_AREA_OFFSET_1, 4);
    if (ret != 0) {
        LOG_ERR("Failed to load program with new ECC count: %d", ret);
        return ret;
    }

    ret = program_execute_and_wait(device->config.spi_dev, page, NULL);
    if (ret != 0) {
        LOG_ERR("Failed to execute program and wait: %d", ret);
        return ret;
    }

    LOG_INF("Updated ECC count for page %u to %u", page, ecc_count_indicator);

    return 0; 
}


/* Read a portion of a page. ECC must be handled by the NAND
 * implementation. Returns 0 on sucess or -1 if an error occurs. If an
 * uncorrectable ECC error occurs, return -1 and set err to E_ECC.
 */
int dhara_nand_read(const struct dhara_nand *n, dhara_page_t p, size_t offset, size_t length,
                    uint8_t *data, dhara_error_t *err)
{
    LOG_DBG("Read, page=%u, offset=%zu, length=%zu", p, offset, length);
    __ASSERT(p < n->num_blocks * (1 << n->log2_ppb), "Page out of range");
    spi_nand_flash_device_t *dev = CONTAINER_OF(n, spi_nand_flash_device_t, dhara_nand);
    int ret;
    uint8_t status;

    
    ret = read_page_and_wait(dev, p, &status);
    if(ret != 0){
        LOG_ERR("error in dhara nand read");
        return -1;
    }

    if (is_ecc_error(status)) {
        LOG_ERR("ECC error on page %u", p);
        dhara_set_error(err, DHARA_E_ECC);
        increase_ECC_counter(dev, p);
        return -1;
    }

    ret = spi_nand_read(dev->config.spi_dev, data, offset, length);
    if (ret != 0) {
        LOG_ERR("Failed to read data from page %u", offset);
        return -1;
    }

    //Log the data read, 40 bytes per line
    // LOG_INF("Data read from page in nand.c %u, offset %zu:", p, offset);
    // for (size_t i = 0; i < length; i++) {
    //     if (i % 40 == 0 && i != 0) {
    //         LOG_INF("");  // New line every 40 bytes, but not at the start
    //     }
    //     printk("%02X ", data[i]);  // Using printk for continuous output on the same line
    // }
    // if (length % 40 != 0) {
    //     LOG_INF("");  // Ensure ending on a new line if not already done
    // }

    return 0;
}




/* Read a page from one location and reprogram it in another location.
 * This might be done using the chip's internal buffers, but it must use
 * ECC.
 */
int dhara_nand_copy(const struct dhara_nand *n, dhara_page_t src, dhara_page_t dst, dhara_error_t *err)
{
    LOG_DBG("Copy, src=%u, dst=%u", src, dst);
    spi_nand_flash_device_t *dev = CONTAINER_OF(n, spi_nand_flash_device_t, dhara_nand);
    int ret;
    uint8_t status;

   
    ret = read_page_and_wait(dev, src, &status);
    if (ret != 0) {
        LOG_ERR("Failed to read page %u", src);
        return -1;
    }

    
    if (is_ecc_error(status)) {
        LOG_ERR("Copy, ECC error detected");
        dhara_set_error(err, DHARA_E_ECC);
        increase_ECC_counter(dev, src);
        return -1;
    }

 
    ret = spi_nand_write_enable(dev->config.spi_dev);
    if (ret != 0) {
        LOG_ERR("Failed to enable write");
        return -1;
    }

    ret = program_execute_and_wait(dev, dst, &status);
    if (ret != 0) {
        LOG_ERR("Program execute failed");
        return -1;
    }

    // Check for programming failure
    if ((status & STAT_PROGRAM_FAILED) != 0) {
        LOG_INF("Copy, program failed");
        dhara_set_error(err, DHARA_E_BAD_BLOCK);
        return -1;
    }

    return 0;
}
