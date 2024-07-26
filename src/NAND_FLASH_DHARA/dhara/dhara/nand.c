//This file lives in the dhara folder (git clone https://github.com/dlbeer/dhara.git)
//rename to nand.c

/**
 * @file nand.c
 * @brief Conection between the  communication layer and the DHARA FTL
 *
 * This file provides the missing implementations that are zephyr framework specific to
 * the dhara flash translation layer (FTL)
 * Author: [Denis Buckingham]
 * Date: [15.03.2024]
 */

#include "nand.h"
#include "../../inc/nand_oper.h" 
#include "../../inc/nand_top_layer.h"//for the nand_flash_device_t


#include <string.h>
#include <stdlib.h>


#define ERASE_COUNTER_SPARE_AREA_OFFSET 16


size_t Erase_counter_FLAG = 0;
dhara_page_t Erased_block = 0;

size_t Delta_ECC_counter = 0;
size_t Initial_ECC_counter = 0;
uint32_t Total_ECC_counter = 0;

uint32_t erase_count_indicator = 0;

uint8_t spare_area_buffer[8];

static uint8_t load_buffer[4200];


//defined in 

/** @brief waiting for finished transaction
 * check repeatedly for the status register
 *
 * defined in the nand.c between nand_oper and dhara
 * used in the top layer and nand.c
 *
 * @param[out] status_out status register content of current transaction
 * @return 0 on success, -1 if the read out of the register failed.
 */
static int wait_for_ready_nand(uint8_t *status_out)
{
    while (true) {
        uint8_t status;
        int err = nand_read_register(REG_STATUS, &status);
        if (err != 0) {
            my_nand_handle->log("Error reading NAND status register",true ,false ,0);
            return -1; 
        }

        if ((status & STAT_BUSY) == 0) {
            if (status_out) {
                *status_out = status;
            }
            break;
        }
    }

    return 0; // Success
}



/**
 * @brief Read a NAND flash page and wait for the operation to complete.
 * 
 * This function initiates a read operation for a specified page in the NAND flash
 * and waits until the device is ready or until an error occurs. It leverages the
 * `nand_read_page` function to perform the read operation and checks the device's
 * status by calling `wait_for_ready_nand`.
 * 
 * @param device A pointer to the nand_flash_device_t structure representing the NAND flash device.
 * @param page The page number to read from the NAND flash.
 * @param status_out Pointer to a variable where the status register's value will be stored upon successful read. 
 *                   Can be NULL if the status is not needed.
 * 
 * @return 0 on success, -1 on failure.
 */
static int read_page_and_wait(struct nand_flash_device_t *device, uint32_t page, uint8_t *status_out)
{
    int err;
    err = nand_read_page(page); 
    if (err != 0) {
        my_nand_handle->log("Failed to read page",true ,true ,page);
        return -1;
    }

    return wait_for_ready_nand(status_out);
}


/**
 * @brief Program a NAND flash page and wait for the operation to complete.
 * 
 * This function initiates a program operation for a specified page in the NAND flash
 * and waits until the device is ready or until an error occurs. It leverages the
 * `nand_program_execute` function to perform the program operation and checks the device's
 * status by calling `wait_for_ready_nand`.
 * 
 * The function ensures that the programming operation is executed correctly by monitoring
 * the NAND flash device's status register after the operation.
 * 
 * @param device A pointer to the nand_flash_device_t structure representing the NAND flash device.
 * @param page The page number where the data will be programmed in the NAND flash.
 * @param status_out Pointer to a variable where the status register's value will be stored upon successful programming. 
 *                   Can be NULL if the status is not needed.
 * 
 * @return 0 on success, -1 on failure.
 */
static int program_execute_and_wait(struct nand_flash_device_t *device, uint32_t page, uint8_t *status_out)
{
    int err;

    err = nand_program_execute(page);
    if (err != 0) {
        my_nand_handle->log("Failed to execute program on page",true ,true ,page);
        return -1;
    }

    return wait_for_ready_nand(status_out);
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
    nand_flash_device_t *dev = CONTAINER_OF(n, nand_flash_device_t, dhara_nand);//struct necessary?

    dhara_page_t first_block_page = b * (1 << n->log2_ppb);
    uint16_t bad_block_indicator;
    int ret;

    ret = read_page_and_wait(dev, first_block_page, NULL);
    if (ret != 0) {
        my_nand_handle->log("Error reading page",true ,true ,first_block_page);
        return 1; // Assume bad block on error
    }

    // Assuming nand_read function reads 'len' bytes starting from 'offset' to 'buf'
    // And assuming the OOB data can be accessed directly following the main data area
    ret = nand_read((uint8_t *)&bad_block_indicator, dev->page_size, 2);
    if (ret != 0) {
        my_nand_handle->log("Failed to read bad block indicator, err",true ,true ,ret);
        return 1; // Assume bad block on error
    }

    //LOG_DBG("Block=%u, Page=%u, Indicator=%04x", b, first_block_page, bad_block_indicator);
    if(bad_block_indicator == 0x0000){my_nand_handle->log("Bad_Block on Block=",false ,true ,b);}
    return bad_block_indicator == 0x0000;
}



//TODO check in datasheet process, erase counter has not to be increased
void dhara_nand_mark_bad(const struct dhara_nand *n, dhara_block_t b)
{
    nand_flash_device_t *dev = CONTAINER_OF(n, nand_flash_device_t, dhara_nand);
    int ret;

    dhara_page_t first_block_page = b * (1 << n->log2_ppb);
    uint16_t bad_block_indicator = 0;
    //LOG_DBG("mark_bad, block=%u, page=%u, indicator = %04x", b, first_block_page, bad_block_indicator);

    ret = nand_write_enable();
    if (ret) {
        my_nand_handle->log("Failed to enable write, error",true ,true ,ret);
        return;
    }

    ret = nand_erase_block(first_block_page);
    if (ret != 0) {
        my_nand_handle->log("Failed to erase block, error",true ,true ,ret);
        return;
    }

    ret = nand_write_enable();
    if (ret != 0) {
        my_nand_handle->log("Failed to enable write, error",true ,true ,ret);
        return;
    }

    ret = nand_program_load((uint8_t *)&bad_block_indicator, dev->page_size, 2);
    if (ret != 0) {
        my_nand_handle->log("Failed to program load, error",true ,true ,ret);
        return;
    }

    ret = program_execute_and_wait(dev, first_block_page, NULL);
    if (ret != 0) {
        my_nand_handle->log("Failed to execute program and wait, error",true ,true ,ret);
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
    //LOG_INF("erase_block, block=%u", b);
    struct nand_flash_device_t *dev = CONTAINER_OF(n, struct nand_flash_device_t, dhara_nand);
    int ret;
    

    dhara_page_t first_block_page = b * (1 << n->log2_ppb);
    uint8_t status;

    //first read out the flags and store them, they will be written to the page
    ret = read_page_and_wait(dev, first_block_page, NULL);
    if (ret) {
        my_nand_handle->log("Failed to read page",true ,true ,first_block_page);
        return -1;
    }

    my_nand_handle->log("Current block",false ,true ,b);
    // uint64_t uptime = k_uptime_get(); // Get the system uptime in milliseconds

    // // Convert uptime to hours, minutes, seconds, and milliseconds
    // uint32_t ms = uptime % 1000;
    // uptime /= 1000;
    // uint32_t sec = uptime % 60;
    // uptime /= 60;
    // uint32_t min = uptime % 60;
    // uptime /= 60;
    // uint32_t hour = uptime;

    // // Create the timestamp string
    // char timestamp[20];
    // snprintf(timestamp, sizeof(timestamp), "%02u:%02u:%02u:%03u", hour, min, sec, ms);

    // // Create the log message with the timestamp
    // char log_message[256];
    // snprintf(log_message, sizeof(log_message), "%s - Current block: ", timestamp);

    // // Call the original log function
    // my_nand_handle->log(log_message, false, true, b);
    /////////////////////////           HEALTH MONITORING START (OPTIONAL)        ///////////////////////////////////

#ifdef CONFIG_HEALTH_MONITORING
    //extract the counters, that are after erasure programmed back to the first page of a block
    uint32_t ecc_count_indicator = 0;
    ret = nand_read(spare_area_buffer, dev->page_size + ERASE_COUNTER_SPARE_AREA_OFFSET, 8);
    if (ret != 0) {
        my_nand_handle->log("Failed to read from spare area", true, true, ret);
        return ret;
    }

    // Extract the erase count indicator and ecc counter from the buffer
    memcpy(&erase_count_indicator, spare_area_buffer, 4);
    memcpy(&ecc_count_indicator, spare_area_buffer + 4, 4);
    
    my_nand_handle->log("Current erase count",false ,true ,erase_count_indicator);
    erase_count_indicator++;
    if (erase_count_indicator == 0) { 
        erase_count_indicator++; 
    }


    //check if there was an old ECC counter already and adjust to that one
    if(ecc_count_indicator != 0xFFFFFFFF){
        if(Initial_ECC_counter == 0 ){Initial_ECC_counter = ecc_count_indicator;}//Initializing counter
        if(ecc_count_indicator > Initial_ECC_counter + Delta_ECC_counter){//we found a counter value in the NAND flash (larger than the one we locally stored since the last start up)
            Initial_ECC_counter = ecc_count_indicator;//thus adjusting the Initial one
        }
        Total_ECC_counter = Initial_ECC_counter + Delta_ECC_counter;
        my_nand_handle->log("Current total ECC faults found",false ,true ,Total_ECC_counter);
    }
    
    Erase_counter_FLAG = 1; //setting flag to make sure, that a page is only written to once, not twice partly
    Erased_block = first_block_page;

#endif // CONFIG_HEALTH_MONITORING
    /////////////////////////           HEALTH MONITORING END (OPTIONAL)        ///////////////////////////////////

    ret = nand_write_enable();
    if (ret != 0) {
        my_nand_handle->log("Failed to enable write, error",true ,true ,ret);
        return -1;
    }

    ret = nand_erase_block(first_block_page);
    if (ret != 0) {
        my_nand_handle->log("Failed to erase block, error",true ,true ,ret);
        return -1;
    }

    ret = wait_for_ready_nand( &status);
    if (ret != 0) {
        my_nand_handle->log("Failed to wait for ready, error",true ,true ,ret);
        return -1;
    }

    if ((status & STAT_ERASE_FAILED) != 0) {
        dhara_set_error(err, DHARA_E_BAD_BLOCK);
        my_nand_handle->log("Erasing failed, indicated by status register",true ,false ,ret);
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
    //LOG_DBG("prog, page=%u", p);
    nand_flash_device_t *dev = CONTAINER_OF(n, nand_flash_device_t, dhara_nand);
    int ret;
    uint8_t status;
    uint16_t used_marker = 0;
    

    ret = read_page_and_wait(dev, p, NULL);//Initiate page read operation to cache. Reads the specified page from NAND to the device's internal cache.
    if (ret) {
        my_nand_handle->log("Failed to read page",true ,true ,p);
        return -1;
    }

    ret = nand_write_enable();//Enable writing on the NAND device.
    if (ret) {
        my_nand_handle->log("Failed to enable write, error",true ,true ,ret);
        return -1;
    }
    memset(load_buffer, 0xFF, 4200);
    memcpy(load_buffer, data, dev->page_size);
    memcpy(load_buffer + dev->page_size + 2, &used_marker, sizeof(used_marker));


    /////////////////////////           HEALTH MONITORING START (OPTIONAL)        ///////////////////////////////////
#ifdef CONFIG_HEALTH_MONITORING
    //we store the erase count indicator into the spare area
    if(Erase_counter_FLAG && Erased_block == p){
        
        //we store the ECC counter and erase counter on the first page of the block
        memcpy(spare_area_buffer, &erase_count_indicator, 4);
        memcpy(spare_area_buffer + 4, &Total_ECC_counter, 4);

        Erase_counter_FLAG = 0;

        memcpy(load_buffer + dev->page_size + ERASE_COUNTER_SPARE_AREA_OFFSET, spare_area_buffer, 8);
        ret = nand_program_load(load_buffer, 0, dev->page_size + sizeof(used_marker) + 2 + ERASE_COUNTER_SPARE_AREA_OFFSET + 8);//put a flag there
        if (ret) {
            my_nand_handle->log("Failed to load erase counter, error",true ,true ,ret);
            return -1;
        }
    }

    ret = program_execute_and_wait(dev, p, &status);//Execute a program operation. Commits the data previously loaded into the device's cache to the NAND array
    if (ret) {
        my_nand_handle->log("Failed to execute program, error",true ,true ,ret);
        return -1;
    }

    if ((status & STAT_PROGRAM_FAILED) != 0) {
        my_nand_handle->log("prog failed, page",true ,true ,p);
        dhara_set_error(err, DHARA_E_BAD_BLOCK);
        return -1;
    }
    

    return 0;
#endif //CONFIG_HEALTH_MONITORING
    /////////////////////////           HEALTH MONITORING END (OPTIONAL)        ///////////////////////////////////

    
    ret = nand_program_load(load_buffer, 0, dev->page_size + sizeof(used_marker) + 2);
    if (ret != 0) {
        my_nand_handle->log("Failed to program load, error", true, true, ret);
        return -1;
    }

    ret = program_execute_and_wait(dev, p, &status);//Execute a program operation. Commits the data previously loaded into the device's cache to the NAND array
    if (ret) {
        my_nand_handle->log("Failed to execute program, error",true ,true ,ret);
        return -1;
    }

    if ((status & STAT_PROGRAM_FAILED) != 0) {
        my_nand_handle->log("prog failed, page",true ,true ,p);
        dhara_set_error(err, DHARA_E_BAD_BLOCK);
        return -1;
    }
    

    return 0;
}




int dhara_nand_is_free(const struct dhara_nand *n, dhara_page_t p)
{
    nand_flash_device_t *dev = CONTAINER_OF(n, nand_flash_device_t, dhara_nand);
    int ret;
    uint16_t used_marker = 0;

    ret = read_page_and_wait(dev, p, NULL);
    if (ret) {
        my_nand_handle->log("Failed to read page",true ,true ,p);
        return 0; 
    }

    ret = nand_read((uint8_t *)&used_marker, dev->page_size + 2, 2);
    if (ret) {
        my_nand_handle->log("Failed to read OOB area for page",true ,true ,p);
        return 0; 
    }

    //LOG_DBG("Is free, page=%u, used_marker=%04x", p, used_marker);
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
    //LOG_DBG("Read, page=%u, offset=%zu, length=%zu", p, offset, length);
    __ASSERT(p < n->num_blocks * (1 << n->log2_ppb), "Page out of range");
    nand_flash_device_t *dev = CONTAINER_OF(n, nand_flash_device_t, dhara_nand);
    int ret;
    uint8_t status;

    
    ret = read_page_and_wait(dev, p, &status);
    if(ret != 0){
        my_nand_handle->log("error in dhara nand read",true ,false ,0);
        return -1;
    }

    if (is_ecc_error(status)) {
        my_nand_handle->log("ECC error on page",true ,true ,p);
        dhara_set_error(err, DHARA_E_ECC);
        Delta_ECC_counter++;
        //increase_ECC_counter(dev, p);
        return -1;
    }

    ret = nand_read(data, offset, length);
    if (ret != 0) {
        my_nand_handle->log("Failed to read data from page",true ,true ,offset);
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
    //LOG_DBG("Copy, src=%u, dst=%u", src, dst);
    nand_flash_device_t *dev = CONTAINER_OF(n, nand_flash_device_t, dhara_nand);
    int ret;
    uint8_t status;

   
    ret = read_page_and_wait(dev, src, &status);
    if (ret != 0) {
        my_nand_handle->log("Failed to read page",true ,true ,src);
        return -1;
    }

    
    if (is_ecc_error(status)) {
        my_nand_handle->log("Copy, ECC error detected",true ,false ,0);
        dhara_set_error(err, DHARA_E_ECC);
        //increase_ECC_counter(dev, src);
        Delta_ECC_counter++;
        return -1;
    }

 
    ret = nand_write_enable();
    if (ret != 0) {
        my_nand_handle->log("Failed to enable write",true ,false ,0);
        return -1;
    }

    ret = program_execute_and_wait(dev, dst, &status);
    if (ret != 0) {
        my_nand_handle->log("Program execute failed",true ,false ,0);
        return -1;
    }

    // Check for programming failure
    if ((status & STAT_PROGRAM_FAILED) != 0) {
        my_nand_handle->log("Copy, program failed",false ,false ,0);
        dhara_set_error(err, DHARA_E_BAD_BLOCK);
        return -1;
    }

    return 0;
}
