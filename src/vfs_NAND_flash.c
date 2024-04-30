#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>                                                                                                                                                     
#include <zephyr/drivers/spi.h>
#include <zephyr/devicetree.h>
#include <zephyr/fs/fs.h>
#include <zephyr/storage/disk_access.h>
#include <ff.h>

#include "vfs_NAND_flash.h"
#include "diskio_nand.h"


LOG_MODULE_REGISTER(vfs_NAND_flash, CONFIG_LOG_DEFAULT_LEVEL);

FATFS fat_fs;

/* FAT fs mount info */
static struct fs_mount_t nand_mount_fat = {
    .type = FS_FATFS,
    .fs_data = &fat_fs,
    .storage_dev = DEVICE_DT_GET(DT_BUS(DT_NODELABEL(spidev))),  // This should match the name of your disk registered
    .mnt_point = "/NAND",       // Mount point in the filesystem
    //.flags = FS_MOUNT_FLAG_USE_DISK_ACCESS
};



void mount_nand_fs(void) {
    int ret;

    

    // ret = disk_nand_init();
    // if (ret) {
    //     LOG_ERR("NAND disk initialization failed with error: %d", ret);
    //     // Handle initialization failure
    // }else{
    //     LOG_INF("Successful disk initialized, message %d", ret);//TODO remove and uncomment in diskio_nand.c ==> automatic initialization on start up
    // }
    

    // Attempt to mount the file system
    ret = fs_mount(&nand_mount_fat);
    if (ret) {
        LOG_ERR("Failed to mount NAND FS (%d)", ret);
    } else {
        LOG_INF("NAND FS mounted");
    }
}