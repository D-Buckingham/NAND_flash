//#include <fs/fs.h>
#include "../inc/nand_top_layer.h"
#include "zephyr/storage/disk_access.h"
#include "../inc/diskio_nand.h"

#include <zephyr/device.h>
#include <zephyr/types.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h> 
#include <zephyr/devicetree.h>
#include <errno.h>



LOG_MODULE_REGISTER(diskio_nand, CONFIG_LOG_DEFAULT_LEVEL);

int nand_disk_access_init(struct disk_info *disk);
int nand_disk_access_status(struct disk_info *disk);
int nand_disk_access_read(struct disk_info *disk, uint8_t *data_buf, uint32_t start_sector, uint32_t num_sector);
int nand_disk_access_write(struct disk_info *disk, const uint8_t *data_buf, uint32_t start_sector, uint32_t num_sector);
int nand_disk_access_ioctl(struct disk_info *disk, uint8_t cmd, void *buff);

/* Disk operations for NAND flash */
static const struct disk_operations nand_disk_ops = {
    .init = nand_disk_access_init,
    .status = nand_disk_access_status,
    .read = nand_disk_access_read,
    .write = nand_disk_access_write,
    .ioctl = nand_disk_access_ioctl
};



//static struct k_mutex disk_mutex;

/* Disk information structure required by Zephyr */
struct disk_info nand_disk = {
    .name = "NAND",
    .ops = &nand_disk_ops,
    .dev = DEVICE_DT_GET(DT_BUS(DT_NODELABEL(nand_device)))
};






int nand_disk_access_init(struct disk_info *disk) {
    // disk->name = "NAND_DISK";
    // disk->ops = &nand_disk_ops;
    // disk->dev = DEVICE_DT_GET(DT_BUS(DT_NODELABEL(nand_device)));
    int ret = nand_flash_init_device(&device_handle);
    return ret;
}

int nand_disk_access_status(struct disk_info *disk) {
    //if (!device_is_ready(disk->dev)) {
    if (!device_is_ready(nand_disk.dev)) {
        LOG_ERR("device is not ready");
        return DISK_STATUS_UNINIT;
    }
    return DISK_STATUS_OK;
}

int nand_disk_access_read(struct disk_info *disk, uint8_t *data_buf, uint32_t start_sector, uint32_t num_sector) {
    LOG_DBG("nand_disk_access_read - disk=%s, start_sector=%u, num_sector=%u", nand_disk.name, start_sector, num_sector);
    uint32_t sector_size;
    int ret = nand_flash_get_sector_size(device_handle, &sector_size);
    if (ret < 0) {
        LOG_ERR("Error getting sector size: %d", ret);
        return -EIO;
    }

    for (uint32_t i = 0; i < num_sector; ++i) {
        ret = nand_flash_read_sector(device_handle, data_buf + i * sector_size, start_sector + i);
        if (ret < 0) {
            LOG_ERR("Failed to read sector %d: %d", start_sector + i, ret);
            return -EIO;
        }
    }
    return 0;

}

int nand_disk_access_write(struct disk_info *disk, const uint8_t *data_buf, uint32_t start_sector, uint32_t num_sector) {
    LOG_DBG("nand_disk_access_write - disk=%s, start_sector=%u, num_sector=%u", nand_disk.name, start_sector, num_sector);
    uint32_t sector_size;
    int ret = nand_flash_get_sector_size(device_handle, &sector_size);
    if (ret < 0) {
        LOG_ERR("Error getting sector size: %d", ret);
        return -EIO;
    }

    for (uint32_t i = 0; i < num_sector; ++i) {
        ret = nand_flash_write_sector(device_handle, data_buf + i * sector_size, start_sector + i);
        if (ret < 0) {
            LOG_ERR("Failed to write sector %d: %d", start_sector + i, ret);
            return DISK_STATUS_WR_PROTECT;
        }
    }
    return 0;

}

int nand_disk_access_ioctl(struct disk_info *disk, uint8_t cmd, void *buff) {
    int ret = 0;

    switch (cmd) {
        case DISK_IOCTL_GET_SECTOR_COUNT:
            // Assuming that the capacity function requires the device handle and a pointer to store the result
            ret = nand_flash_get_capacity(device_handle, (uint32_t *)buff);
            LOG_INF("The sector count (capacity) is: %d", *((uint32_t *)buff));
            if (ret < 0) {
                LOG_ERR("Failed to get capacity: error %d", ret);
                return -EIO;
            }
            break;

        case DISK_IOCTL_GET_SECTOR_SIZE:
            // Assuming that the sector size function requires the device handle and a pointer to store the result
            ret = nand_flash_get_sector_size(device_handle, (uint32_t *)buff);
            LOG_INF("The sector size is: %d", *((uint32_t *)buff));
            if (ret < 0) {
                LOG_ERR("Failed to get sector size: error %d", ret);
                return -EIO;
            }
            break;

        case DISK_IOCTL_CTRL_SYNC:
            // Assuming that the sync function only requires the device handle and returns status
            ret = nand_flash_sync(device_handle);
            if (ret < 0) {
                LOG_ERR("Failed to sync device: error %d", ret);
                return -EIO;
            }
            break;

        case DISK_IOCTL_GET_ERASE_BLOCK_SZ:
            *(uint32_t *)buff = device_handle->block_size;
		    break;

        default:
            LOG_ERR("Unsupported IOCTL command: %d", cmd);
            return -EINVAL;
    }
    return 0;
}


int disk_nand_init(void)
{
    //k_mutex_lock(&disk_mutex, K_FOREVER);
    LOG_INF("DISK LAYER: Initializing NAND disk");
    int ret = disk_access_register(&nand_disk);  // Register the disk
    if (ret) {
        LOG_ERR("Failed to register NAND disk");
        return ret;
    }
    //k_mutex_unlock(&disk_mutex);
    if(sys_dnode_is_linked(&nand_disk.node)){
        LOG_INF("DISK LAYER: Node successfully linked!");
    }else{
        LOG_ERR("no linked node for disk");
    }
    return 0;
}

int disk_nand_uninit(void)
{
    int ret = disk_access_unregister(&nand_disk);  // Register the disk
    if (ret) {
        LOG_ERR("Failed to unregister NAND disk");
        return ret;
    }
    return 0;
}



SYS_INIT(disk_nand_init, APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);