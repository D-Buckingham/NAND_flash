#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <assert.h>

#include    "test_spi_nand_top_layer.h"
#include    "spi_nand_oper_tests.h"
#include    "nand_top_layer.h"
#include    "../vfs_NAND_flash.h"
#include    "../SPI_NAND_DHARA/diskio_nand.h"
#include    "../SPI_NAND_DHARA/spi_nand_oper.h"


LOG_MODULE_REGISTER(test_main_top, CONFIG_LOG_DEFAULT_LEVEL);

//is a device connected?
//get device through dhara and check if it is there
int top_device_connected(void){
    int res;
    struct fs_statvfs sbuf;
    uint32_t sector_size;

    LOG_INF("Top Test 1: Checking initialization on every level");

    //device connected?
    const struct spi_dt_spec spidev_dt = spi_nand_init();//gets pointer to device from DT
    if (!device_is_ready(spidev_dt.bus)) {
        LOG_ERR("Device not ready on SPI level");
        return -1;
    }else{
        LOG_INF("Device correctly retrieved from device tree");
    }


    //check if the communication works by reading out the device ID
    res = test_IDs_spi_nand(&spidev_dt);
    if (res != 0) {
        LOG_ERR("Device & Manufacturer ID test failed");
        return -1;
    }


    //get sector size to validate if dhara map layer works
    spi_nand_flash_device_t *nand_flash_device_handle = NULL;
    spi_nand_flash_config_t nand_flash_config = {
        .spi_dev = &spidev_dt,
    };
    spi_nand_flash_init_device(&nand_flash_config, &nand_flash_device_handle);
    res = spi_nand_flash_get_sector_size(nand_flash_device_handle, &sector_size);
    if(res != 0){
        LOG_ERR("Unable to get sector size, error: %d", res);
        return -1;
    }  else{
        LOG_INF("size of sector is: %u", sector_size);
    }


    //disk properly connected?
    if(nand_disk_access_status(&nand_disk) != DISK_STATUS_OK){
        LOG_ERR("Failed to check status!");
        return -1;
    }else{
        LOG_INF("Disk access status = ok");
    }


    //if flash correctly mounted
    res = fs_statvfs(nand_mount_fat.mnt_point, &sbuf);
	if (res < 0) {
		LOG_ERR("FAIL: statvfs: %d\n", res);
        return -1;
	}else{
    LOG_PRINTK("%s: bsize = %lu ; frsize = %lu ;"
		   " blocks = %lu ; bfree = %lu\n",
		   nand_mount_fat.mnt_point,
		   sbuf.f_bsize, sbuf.f_frsize,
		   sbuf.f_blocks, sbuf.f_bfree);
    }

    
    return 0;
}



//Can I create a folder?


//Can I create a file?


//Can I read the file?


//How to store a big file? Stored over multiple blocks?


//Adding data to big file, how is it changed on the actual device? Inspect mapping of dhara


//Change data of a file, how is this process handled in dhara/ on the actual device?


//How is a file deleted?


//Write to a 1/8 of the flash, how is it done?


//How are multiple small files stored in dhara?



//Main function in which the other ones are called
int test_all_main_nand_tests(void){

    //is the device on each layer connected/initialized?
    int ret = top_device_connected();
    if(ret == 0){
        LOG_INF("Test 1 passed!");
    }
    return 0;
}
