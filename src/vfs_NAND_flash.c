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

/* FAT fs mount info */
FATFS fat_fs;
struct fs_mount_t nand_mount_fat = {
    .type = FS_FATFS,
    .fs_data = &fat_fs,
    .flags = FS_MOUNT_FLAG_USE_DISK_ACCESS,//FS_MOUNT_FLAG_NO_FORMAT//FS_MOUNT_FLAG_USE_DISK_ACCESS,
    .storage_dev = (void *) "NAND",  // This should match the name of your disk registered
    .mnt_point = "/NAND:"       // Mount point in the filesystem
};

void mount_nand_fs(void) {
    int ret;

    // Attempt to mount the file system
    ret = fs_mount(&nand_mount_fat);
    if (ret) {
        LOG_ERR("Failed to mount NAND FS (%d)", ret);
    } else {
        LOG_INF("NAND FS mounted");
    } 
}