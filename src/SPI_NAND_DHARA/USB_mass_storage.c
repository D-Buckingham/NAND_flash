#include <zephyr/usb/usb_device.h>
#include <zephyr/fs/fs.h>
#include <zephyr/fs/fcb.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/usb/class/usbd_msc.h>
#include <zephyr/usb/usbd.h>
#include "zephyr/storage/disk_access.h"

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


// static struct usbd_contex *sample_usbd;

// static int enable_usb_device_next(void)
// {
// 	int err;

// 	// sample_usbd = sample_usbd_init_device(NULL);
// 	// if (sample_usbd == NULL) {
// 	// 	LOG_ERR("Failed to initialize USB device");
// 	// 	return -ENODEV;
// 	// }

// 	err = usbd_enable(sample_usbd);
// 	if (err) {
// 		LOG_ERR("Failed to enable device support");
// 		return err;
// 	}

// 	LOG_DBG("USB device support enabled");

// 	return 0;
// }



#include <stdint.h>

#include <zephyr/device.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/sys/iterable_sections.h>



#define ZEPHYR_PROJECT_USB_VID		0x2fe3




int initialize_mass_storage_nand(void)
{

    int ret;
    k_msleep(100);
    LOG_INF("USB Mass Storage on NAND Flash");
    //rc = enable_usb_device_next();
    ret = usb_enable(NULL);
    if (ret != 0) {
        LOG_ERR("Failed to enable USB");
        return -1;
    }

    // /* Set the USB Mass Storage parameters */
    // ret = usbd_msc_register("NAND", NULL);
    // if (ret) {
    //     printk("Failed to register USB MSC\n");
    //     return -1;
    // }


    // rc = msc_register_cb(&msc_callbacks);
    // if (rc != 0) {
    //     LOG_ERR("Failed to register MSC callbacks");
    //     return -1;
    // }
    return 0;

    LOG_INF("USB Mass Storage initialized");
}
