
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>                                                                                                                                                     
#include "zephyr/storage/disk_access.h"

#include "diskio_nand.h"
#include "test_disk_access.h"
#include <zephyr/devicetree.h>

#include <string.h> 
#include <stdlib.h>


LOG_MODULE_REGISTER(test_disk_access, CONFIG_LOG_DEFAULT_LEVEL);

//static const struct disk_info *nand_disk = NULL;
static uint8_t pattern_buf[2048];

//test Initialized, status, read
int test_disk_initialize_status_read(const struct disk_info *nand_disk){
   
    if(nand_disk_access_init(nand_disk) != 0){
        LOG_ERR("failed to initialize NAND DISK!");
    }

    if(nand_disk_access_status(nand_disk) != DISK_STATUS_OK){
        LOG_ERR("Failed to check status!");
    }

    if(nand_disk_access_read(nand_disk, &pattern_buf, 1, 1) != 0){
        LOG_ERR("Failed to read out buffer on disk level");
    }

    LOG_INF("Contents of read out disk level (800 bytes):");
    for (int i = 0; i < 800; i++) {
        if (i % 40 == 0 && i != 0) {
            LOG_INF("");  
        }
        printk("%02X ", pattern_buf[i]);
    }
    LOG_INF("\n... (plus %d more bytes)", 2048 - 800);

    return 0;

}
