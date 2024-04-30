
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

#define PATTERN_SEED    0x12345678

static struct k_mutex disk_mutex;


static int check_buffer(uint32_t seed, const uint8_t *src, size_t count)
{
    int ret = 0;
    srand(seed);
    for (size_t i = 0; i < count; ++i) {
        uint8_t val = src[i];
        uint8_t expected = rand() & 0xFF; 
        if (val != expected) {
            if(i == 600){
            LOG_ERR("Mismatch at index %zu: expected 0x%02X, got 0x%02X", i, expected, val);
            }ret = -1;
        }
    }
    return ret;
}

static void fill_buffer(uint32_t seed, uint8_t *dst, size_t count){
    srand(seed);
    for (size_t i = 0; i < count; ++i) {
        dst[i] = rand() & 0xFF;  
        //LOG_INF("Index %zu: 0x%02X", i, dst[i]);
    }
}

//test Initialized, status, read
int test_disk_initialize_status_read(struct disk_info *nand_disk){

    static uint8_t pattern_buf[2048];
    static uint8_t temp_buf[2048];

    memset((void *)pattern_buf, 0x00, sizeof(pattern_buf));
    memset((void *)temp_buf, 0x00, sizeof(temp_buf));
   
    if(nand_disk_access_init(nand_disk) != 0){
        LOG_ERR("failed to initialize NAND DISK!");
    }else{
        LOG_INF("disk access sucessfully initialized");
    }

    if(nand_disk_access_status(nand_disk) != DISK_STATUS_OK){
        LOG_ERR("Failed to check status!");
    }else{
        LOG_INF("Disk access status = ok");
    }

    if(nand_disk_access_read(nand_disk, pattern_buf, 1, 1) != 0){
        LOG_ERR("Failed to read out buffer on disk level");
    }else{
        LOG_INF("reading sector 1");
    }

    LOG_INF("Contents of read out disk level (800 bytes):");
    for (int i = 0; i < 800; i++) {
        if (i % 40 == 0 && i != 0) {
            LOG_INF("");  
        }
        printk("%02X ", pattern_buf[i]);
    }
    LOG_INF("\n... (plus %d more bytes)", 2048 - 800);

    fill_buffer(PATTERN_SEED, pattern_buf, 2048);

    LOG_INF("Initializing write read operation through disk level");

    //write
    if(nand_disk_access_write(nand_disk, pattern_buf, 1, 1)){
      LOG_ERR("Failed to write buffer on disk level");
    }else{
        LOG_INF("reading sector 1");
    }

    //read
    if(nand_disk_access_read(nand_disk, temp_buf, 1, 1) != 0){
        LOG_ERR("Failed to read out buffer on disk level");
    }else{
        LOG_INF("reading sector 1");
    }

    //compare
    check_buffer(PATTERN_SEED, temp_buf, 2048);

    LOG_INF("Contents of write read on disk level (800 bytes):");
    for (int i = 0; i < 800; i++) {
        if (i % 40 == 0 && i != 0) {
            LOG_INF("");  
        }
        printk("%02X ", temp_buf[i]);
    }
    LOG_INF("\n... (plus %d more bytes)", 2048 - 800);

    k_mutex_lock(&disk_mutex, K_FOREVER);

    // Register the disk
    int ret = disk_access_register(nand_disk);
    if (ret) {
        LOG_ERR("Failed to register NAND disk");
        k_mutex_unlock(&disk_mutex); 
        return ret;
    }else{
        LOG_INF("Registered nand disk");
    }

    k_mutex_unlock(&disk_mutex);

    return 0;

}
