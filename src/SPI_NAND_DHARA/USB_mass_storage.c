#include <zephyr/usb/usb_device.h>
#include <zephyr/fs/fcb.h>
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
#include    "vfs_NAND_flash.h"
#include    "diskio_nand.h"
#include    "nand_top_layer.h"
#include "USB_mass_storage.h"


LOG_MODULE_REGISTER(usb_mass, CONFIG_LOG_DEFAULT_LEVEL);



USBD_CONFIGURATION_DEFINE(config_1,
			  USB_SCD_SELF_POWERED,
			  200);
USBD_DESC_LANG_DEFINE(sample_lang);//set to english
USBD_DESC_MANUFACTURER_DEFINE(sample_mfr, "Relab");
USBD_DESC_PRODUCT_DEFINE(sample_product, "NAND");
USBD_DESC_SERIAL_NUMBER_DEFINE(sample_sn, "0123456789abcdef");

USBD_DEVICE_DEFINE(sample_usbd,
		   DEVICE_DT_GET(DT_NODELABEL(zephyr_udc0)),
		   0x2fe3, 0x0008);


USBD_DEFINE_MSC_LUN(NAND, "Relab", "NAND", "0.00");

static int enable_usb_device_next(void)
{
	int err;

	err = usbd_add_descriptor(&sample_usbd, &sample_lang);
	if (err) {
		LOG_ERR("Failed to initialize language descriptor (%d)", err);
		return err;
	}

	err = usbd_add_descriptor(&sample_usbd, &sample_mfr);
	if (err) {
		LOG_ERR("Failed to initialize manufacturer descriptor (%d)", err);
		return err;
	}

	err = usbd_add_descriptor(&sample_usbd, &sample_product);
	if (err) {
		LOG_ERR("Failed to initialize product descriptor (%d)", err);
		return err;
	}

	err = usbd_add_descriptor(&sample_usbd, &sample_sn);
	if (err) {
		LOG_ERR("Failed to initialize SN descriptor (%d)", err);
		return err;
	}

	err = usbd_add_configuration(&sample_usbd, &config_1);
	if (err) {
		LOG_ERR("Failed to add configuration (%d)", err);
		return err;
	}

	err = usbd_register_class(&sample_usbd, "msc_0", 1);
	if (err) {
		LOG_ERR("Failed to register MSC class (%d)", err);
		return err;
	}

	err = usbd_init(&sample_usbd);
	if (err) {
		LOG_ERR("Failed to initialize device support");
		return err;
	}

	err = usbd_enable(&sample_usbd);
	if (err) {
		LOG_ERR("Failed to enable device support");
		return err;
	}

	LOG_DBG("USB device support enabled");

	return 0;
}

int initialize_mass_storage_nand(void)
{

    int ret;
    k_msleep(100);
    LOG_INF("USB Mass Storage on NAND Flash");
    ret = enable_usb_device_next();
    
    if (ret != 0) {
        LOG_ERR("Failed to enable USB");
        return -1;
    }
    LOG_INF("USB Mass Storage initialized");

    return 0;
}
