#include <zephyr/usb/usb_device.h>
#include <zephyr/fs/fs.h>
#include <zephyr/fs/fcb.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/usb/class/usbd_msc.h>
#include <zephyr/usb/usbd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <assert.h>

#include    "nand_top_layer.h"
#include    "../SPI_NAND_DHARA/vfs_NAND_flash.h"
#include    "../SPI_NAND_DHARA/diskio_nand.h"
#include    "../SPI_NAND_DHARA/spi_nand_oper.h"
#include    "../SPI_NAND_DHARA/nand_top_layer.h"

#include "USB_mass_storage.h"

#define CONFIG_USB_MASS_STORAGE_BULK_EP_MPS 512


LOG_MODULE_REGISTER(usb_mass, CONFIG_LOG_DEFAULT_LEVEL);


USBD_DEFINE_MSC_LUN(NAND, "Zephyr", "FlashDisk", "0.00");


// int msc_read(uint32_t lba, uint32_t blk_cnt, uint8_t *buf)
// {
//     struct fs_file_t file;
//     fs_file_t_init(&file);

//     int rc = fs_open(&file, nand_mount_fat.mnt_point, FS_O_READ);
//     if (rc < 0) {
//         LOG_ERR("Failed to open NAND flash: %d", rc);
//         return -EIO;
//     }

//     rc = fs_seek(&file, lba * CONFIG_USB_MASS_STORAGE_BULK_EP_MPS, FS_SEEK_SET);
//     if (rc < 0) {
//         LOG_ERR("Failed to seek NAND flash: %d", rc);
//         fs_close(&file);
//         return -EIO;
//     }

//     rc = fs_read(&file, buf, blk_cnt * CONFIG_USB_MASS_STORAGE_BULK_EP_MPS);
//     if (rc < 0) {
//         LOG_ERR("Failed to read NAND flash: %d", rc);
//         fs_close(&file);
//         return -EIO;
//     }

//     fs_close(&file);
//     return 0;
// }

// int msc_write(uint32_t lba, uint32_t blk_cnt, const uint8_t *buf)
// {
//     struct fs_file_t file;
//     fs_file_t_init(&file);

//     int rc = fs_open(&file, nand_mount_fat.mnt_point, FS_O_WRITE);
//     if (rc < 0) {
//         LOG_ERR("Failed to open NAND flash: %d", rc);
//         return -EIO;
//     }

//     rc = fs_seek(&file, lba * CONFIG_USB_MASS_STORAGE_BULK_EP_MPS, FS_SEEK_SET);
//     if (rc < 0) {
//         LOG_ERR("Failed to seek NAND flash: %d", rc);
//         fs_close(&file);
//         return -EIO;
//     }

//     rc = fs_write(&file, buf, blk_cnt * CONFIG_USB_MASS_STORAGE_BULK_EP_MPS);
//     if (rc < 0) {
//         LOG_ERR("Failed to write NAND flash: %d", rc);
//         fs_close(&file);
//         return -EIO;
//     }

//     fs_close(&file);
//     return 0;
// }

// static const struct mscd_cb msc_callbacks = {
//     .read = msc_read,
//     .write = msc_write,
// };


int initialize_mass_storage_nand(void)
{

    int rc;

    LOG_INF("USB Mass Storage on NAND Flash");

    rc = usb_enable(NULL);
    if (rc != 0) {
        LOG_ERR("Failed to enable USB");
        return -1;
    }

    // rc = msc_register_cb(&msc_callbacks);
    // if (rc != 0) {
    //     LOG_ERR("Failed to register MSC callbacks");
    //     return -1;
    // }
    return 0;

    LOG_INF("USB Mass Storage initialized");
}
