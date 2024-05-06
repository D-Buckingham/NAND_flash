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

static FATFS fat_fs;

/* FAT fs mount info */
static struct fs_mount_t nand_mount_fat = {
    .type = FS_FATFS,
    .fs_data = &fat_fs,
    .flags = FS_MOUNT_FLAG_USE_DISK_ACCESS,//FS_MOUNT_FLAG_NO_FORMAT//FS_MOUNT_FLAG_USE_DISK_ACCESS,
    .storage_dev = (void *) "NAND",  // This should match the name of your disk registered
    .mnt_point = "/NAND:"       // Mount point in the filesystem
};



void mount_nand_fs(void) {
    int ret;

	// struct ext2_cfg ext2_config = {
	// 	.block_size = 2048,
	// 	.fs_size = 0x2000000,
	// 	.bytes_per_inode = 0,
	// 	.volume_name[0] = 0,
	// 	.set_uuid = false,
	// };

    // static MKFS_PARM def_cfg = {
    //     .fmt = FM_ANY | FM_SFD,	/* Any suitable FAT */
    //     .n_fat = 1,		/* One FAT fs table */
    //     .align = 0,		/* Get sector size via diskio query */
    //     .n_root = CONFIG_FS_FATFS_MAX_ROOT_ENTRIES,
    //     .au_size = 0		/* Auto calculate cluster size */
    // };

    // ret = fs_mkfs(FS_FATFS, (uintptr_t)nand_mount_fat.storage_dev, &ext2_config, 0);
    

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