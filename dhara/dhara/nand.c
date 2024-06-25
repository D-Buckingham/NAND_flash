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
#define ERASE_COUNTER_SPARE_AREA_OFFSET 16
#define ECC_COUNTER_SPARE_AREA_OFFSET 36

size_t Erase_counter_FLAG = 0;
dhara_page_t Erased_block = 0;

size_t Delta_ECC_counter = 0;
size_t Initial_ECC_counter = 0;
uint32_t Total_ECC_counter = 0;

uint32_t erase_count_indicator = 0;


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
static int wait_for_ready_nand(uint32_t expected_operation_time_us, uint8_t *status_out)
{

    // Assuming ROM_WAIT_THRESHOLD_US is defined somewhere globally
    if (expected_operation_time_us < ROM_WAIT_THRESHOLD_US) {
        k_busy_wait(expected_operation_time_us); // busy wait for microseconds
    }

    while (true) {
        uint8_t status;
        int err = spi_nand_read_register(REG_STATUS, &status);
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
    err = spi_nand_read_page(page); 
    if (err != 0) {
        LOG_ERR("Failed to read page %u, error: %d", page, err);
        return -1;
    }

    return wait_for_ready_nand(device -> read_page_delay_us, status_out);
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

    err = spi_nand_program_execute(page);
    if (err != 0) {
        LOG_ERR("Failed to execute program on page %u, error: %d", page, err);
        return -1;
    }

    return wait_for_ready_nand( device -> program_page_delay_us, status_out);
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
    ret = spi_nand_read((uint8_t *)&bad_block_indicator, dev->page_size, 2);
    if (ret != 0) {
        LOG_ERR("Failed to read bad block indicator, err: %d", ret);
        return 1; // Assume bad block on error
    }

    LOG_DBG("Block=%u, Page=%u, Indicator=%04x", b, first_block_page, bad_block_indicator);
    if(bad_block_indicator == 0x0000){LOG_INF("Bad_Block on Block=%u, Page=%u", b, first_block_page);}
    return bad_block_indicator == 0x0000;
}



//TODO check in datasheet process, erase counter has not to be increased
void dhara_nand_mark_bad(const struct dhara_nand *n, dhara_block_t b)
{
    spi_nand_flash_device_t *dev = CONTAINER_OF(n, spi_nand_flash_device_t, dhara_nand);
    int ret;

    dhara_page_t first_block_page = b * (1 << n->log2_ppb);
    uint16_t bad_block_indicator = 0;
    LOG_DBG("mark_bad, block=%u, page=%u, indicator = %04x", b, first_block_page, bad_block_indicator);

    ret = spi_nand_write_enable();
    if (ret) {
        LOG_ERR("Failed to enable write, error: %d", ret);
        return;
    }

    ret = spi_nand_erase_block(first_block_page);
    if (ret != 0) {
        LOG_ERR("Failed to erase block, error: %d", ret);
        return;
    }

    ret = spi_nand_write_enable();
    if (ret != 0) {
        LOG_ERR("Failed to enable write, error: %d", ret);
        return;
    }

    ret = spi_nand_program_load((uint8_t *)&bad_block_indicator, dev->page_size, 2);
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
    uint32_t ecc_count_indicator = 0;

    dhara_page_t first_block_page = b * (1 << n->log2_ppb);
    uint8_t status;


    //first read out the flags and store them, they will be written to the page
    ret = read_page_and_wait(dev, first_block_page, NULL);
    if (ret) {
        LOG_ERR("Failed to read page %u", first_block_page);
        return -1;
    }

    // Read the current erase count indicator from the spare area
    ret = spi_nand_read((uint8_t *)&erase_count_indicator, dev->page_size + ERASE_COUNTER_SPARE_AREA_OFFSET, 4);
    if (ret != 0) {
        LOG_ERR("Failed to read erase count from spare area: %d", ret);
        return ret;
    }

    LOG_INF("Current erase count for block at page %u: %u", Erased_block, erase_count_indicator);

    // Increment the erase count
    erase_count_indicator++;
    if (erase_count_indicator == 0){erase_count_indicator++;}


    //read out the ECC counter from the first page of each block
    ret = spi_nand_read((uint8_t *)&ecc_count_indicator, dev->page_size + ECC_COUNTER_SPARE_AREA_OFFSET, 4);
    if (ret != 0) {
        LOG_ERR("Failed to read erase count from spare area: %d", ret);
        return ret;
    }
    if(ecc_count_indicator != 0xFFFFFFFF){
        if(Initial_ECC_counter == 0 ){Initial_ECC_counter = ecc_count_indicator;}//Initializing counter
        if(ecc_count_indicator > Initial_ECC_counter + Delta_ECC_counter){//we found a counter value in the NAND flash (larger than the one we locally stored since the last start up)
            Initial_ECC_counter = ecc_count_indicator;//thus adjusting the Initial one
        }
        Total_ECC_counter = Initial_ECC_counter + Delta_ECC_counter;
        LOG_INF("Current total ECC faults found %u:", Total_ECC_counter);
    }
    


    ret = spi_nand_write_enable();
    if (ret != 0) {
        LOG_ERR("Failed to enable write, error: %d", ret);
        return -1;
    }

    ret = spi_nand_erase_block(first_block_page);
    if (ret != 0) {
        LOG_ERR("Failed to erase block, error: %d", ret);
        return -1;
    }

    ret = wait_for_ready_nand(dev->erase_block_delay_us, &status);
    if (ret != 0) {
        LOG_ERR("Failed to wait for ready, error: %d", ret);
        return -1;
    }

    if ((status & STAT_ERASE_FAILED) != 0) {
        dhara_set_error(err, DHARA_E_BAD_BLOCK);
        LOG_ERR("Erasing failed, indicated by status register");
        return -1;
    }
    Erase_counter_FLAG = 1; //setting flag to make sure, that a page is only written to once, not twice partly
    Erased_block = first_block_page;

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

    ret = spi_nand_write_enable();//Enable writing on the SPI NAND device.
    if (ret) {
        LOG_ERR("Failed to enable write, error: %d", ret);
        return -1;
    }

    ret = spi_nand_program_load(data, 0, dev->page_size);//Load data into the SPI NAND device's cache.
    if (ret) {
        LOG_ERR("Failed to load program, error: %d", ret);
        return -1;
    }

    ret = spi_nand_program_load((uint8_t *)&used_marker, dev->page_size + 2, 2);//put a flag there
    if (ret) {
        LOG_ERR("Failed to load used marker, error: %d", ret);
        return -1;
    }

    if(Erase_counter_FLAG && Erased_block == p){
        ret = spi_nand_program_load((uint8_t *)&erase_count_indicator, dev->page_size + ERASE_COUNTER_SPARE_AREA_OFFSET, 4);//put a flag there
        if (ret) {
            LOG_ERR("Failed to load erase counter, error: %d", ret);
            return -1;
        }
        Erase_counter_FLAG = 0;

        //we store the ECC counter as well since it is the first page
        ret = spi_nand_program_load((uint8_t *)&Total_ECC_counter, dev->page_size + ECC_COUNTER_SPARE_AREA_OFFSET, 4);//put a flag there
        if (ret) {
            LOG_ERR("Failed to load ECC counter, error: %d", ret);
            return -1;
        }

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

    ret = spi_nand_read((uint8_t *)&used_marker, dev->page_size + 2, 2);
    if (ret) {
        LOG_ERR("Failed to read OOB area for page %u", p);
        return 0; 
    }

    LOG_DBG("Is free, page=%u, used_marker=%04x", p, used_marker);
    return used_marker == 0xFFFF; // Check against expected marker value for a free page
}


static int is_ecc_error(uint8_t status)
{
    return (status & STAT_ECC1) != 0 && (status & STAT_ECC0) == 0;
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
        Delta_ECC_counter++;
        //increase_ECC_counter(dev, p);
        return -1;
    }

    ret = spi_nand_read(data, offset, length);
    if (ret != 0) {
        LOG_ERR("Failed to read data from page %u", offset);
        return -1;
    }

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
        //increase_ECC_counter(dev, src);
        Delta_ECC_counter++;
        return -1;
    }

 
    ret = spi_nand_write_enable();
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
