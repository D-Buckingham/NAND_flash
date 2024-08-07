#ifdef CONFIG_HEALTH_MONITORING
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>


#include <assert.h>

#include "../inc/nand_top_layer.h"
#include "../inc/nand_driver.h"
#include "../inc/health_monitoring.h"


#define ROM_WAIT_THRESHOLD_US 1000
#define ERASE_COUNTER_SPARE_AREA_OFFSET 16
#define ECC_COUNTER_SPARE_AREA_OFFSET 20



LOG_MODULE_REGISTER(health_monitoring, CONFIG_LOG_DEFAULT_LEVEL);

//Bad Block Count, percentage of capacity
//Erase Count / Wear Leveling to estimate live cycle, Number of times each block has been erased and programmed
//Program / Erase Cycles, Number of program/erase cycles the flash memory has undergone.
//ECC (Error-Correcting Code) Errors, Indicates the level of data corruption and the effectiveness of error correction. An increasing number of ECC corrections can signal degrading memory cells.



// Structure to hold flash health metrics
struct flash_health_metrics {
    uint32_t bad_block_count;//overall bad block counter
    uint32_t erase_count;//Number of times each block has been erased and programmed
    uint32_t ecc_errors;//Indicates the level of data corruption and the effectiveness of error correction. An increasing number of ECC corrections can signal degrading memory cells
    
};

// Function to initialize and retrieve flash health metrics
void get_flash_health_metrics(struct flash_health_metrics *metrics) {
    // Code to retrieve and populate metrics
    metrics->bad_block_count = read_bad_block_count();
    metrics->erase_count = read_erase_count();
    metrics->ecc_errors = read_ecc_errors();
}



int display_health(){
    LOG_INF("Health Monitoring NAND Flash:");
    struct flash_health_metrics metrics;
    get_flash_health_metrics(&metrics);
    LOG_INF("Total Bad Block Count: %u", metrics.bad_block_count);
    LOG_INF("Mean Erase Count per Block: %u", metrics.erase_count);
    LOG_INF("Total ECC Errors: %u", metrics.ecc_errors);
    return 0;
}


static int wait_for_ready_nand( uint8_t *status_out)
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

    return 0; // Success
}

static int read_page_and_wait( uint32_t page, uint8_t *status_out)
{
    int err;
    err = nand_read_page(page); 
    if (err != 0) {
        LOG_ERR("Failed to read page %u, error: %d", page, err);
        return -1;
    }

    return wait_for_ready_nand(status_out);
}

//checks on first page of each block the total amount of bad blocks out
//spare area 816h 817h


/**
 * @brief Count the number of bad blocks in the NAND flash.
 * 
 * This function iterates through each block, reads the first page of each block,
 * and checks to determine how many bad blocks are there.
 * 
 *
 * @return The total number of bad blocks.
 */
uint32_t read_bad_block_count(void) {
    uint16_t bad_block_indicator;
    uint32_t bad_block_count = 0;
    int ret;

    for (int b = 0; b < device_handle->num_blocks; b++) {
        uint32_t first_block_page = b * (1 << device_handle->dhara_nand.log2_ppb);

        // Read the first page of the block
        ret = read_page_and_wait(first_block_page, NULL);
        if (ret != 0) {
            LOG_ERR("Error reading page: %d", ret);
            // Assume block is bad if read fails
            bad_block_count++;
            continue;
        }

        // Read the bad block indicator from the spare area (0x816 and 0x817)
        ret = nand_read((uint8_t *)&bad_block_indicator, device_handle->page_size, 2);
        if (ret != 0) {
            LOG_ERR("Failed to read bad block indicator from block %u, err: %d", b, ret);
            // Assume block is bad if read fails
            bad_block_count++;
            continue;
        }

        //LOG_INF("Block=%u, Page=%u, Indicator=%04x", b, first_block_page, bad_block_indicator);

        // Check if the block is bad
        if (bad_block_indicator != 0xFFFF) {
            bad_block_count++;
        }
    }

    return bad_block_count;
}


//counter indicating on how many cycles, first spare area
//spare area 816h 820h
uint32_t read_erase_count(void){

    uint32_t erase_count_indicator;
    uint32_t erase_count_mean = 0;
    int ret;

    for (int b = 0; b < device_handle->num_blocks; b++) {
        uint32_t first_block_page = b * (1 << device_handle->dhara_nand.log2_ppb);

        // Read the first page of the block
        ret = read_page_and_wait(first_block_page, NULL);
        if (ret != 0) {
            LOG_ERR("Error reading page: %d", ret);
            // Assume block is bad if read fails
            continue;
        }

        // Read the bad block indicator from the spare area (0x816 and 0x817)
        ret = nand_read((uint8_t *)&erase_count_indicator, device_handle->page_size + ERASE_COUNTER_SPARE_AREA_OFFSET, 4);
        if (ret != 0) {
            LOG_ERR("Failed to read assuming block bad");
            continue;
        }

        //LOG_INF("Block=%u, Page=%u, Indicator=%04x", b, first_block_page, erase_count_indicator);

        // Check if the block is bad
        if (erase_count_indicator != 0x00000000 && erase_count_indicator != 0xFFFFFFFF) {
            erase_count_mean += erase_count_indicator;
        }
    }
    erase_count_mean /= device_handle->num_blocks;
    return erase_count_mean;
}


//spare area 824h
uint32_t read_ecc_errors(void) {
    uint32_t ecc_error_count = 0;
    uint32_t ecc_error_total = 0;
    int ret;

    for (int b = 0; b < device_handle->num_blocks; b++) {
        uint32_t first_block_page = b * (1 << device_handle->dhara_nand.log2_ppb);

        // Read the first page of the block
        ret = read_page_and_wait(first_block_page, NULL);
        if (ret != 0) {
            LOG_ERR("Error reading page: %d", ret);
            // Skip this block if read fails
            continue;
        }

        // Read the ECC error count from the spare area (0x821 to 0x824)
        ret = nand_read((uint8_t *)&ecc_error_count, device_handle->page_size + ECC_COUNTER_SPARE_AREA_OFFSET, 4);
        if (ret != 0) {
            LOG_ERR("Failed to read ECC error count, assuming block bad");
            continue;
        }

        //LOG_INF("Block=%u, Page=%u, ECC Errors=%04x", b, first_block_page, ecc_error_count);

        // Check if the ECC error count is valid
        if (ecc_error_count != 0x00000000 && ecc_error_count != 0xFFFFFFFF) {
            ecc_error_total += ecc_error_count;
        }
    }

    

    return ecc_error_total;
}


//SYS_INIT(display_health, APPLICATION, 60);

#endif //CONFIG_HEALTH_MONITORING