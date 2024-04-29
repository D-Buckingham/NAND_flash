//#include <fs/fs.h>
#include "nand_top_layer.h"
#include "zephyr/storage/disk_access.h"
#include "diskio_nand.h"

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h> 
#include <zephyr/devicetree.h>
#include <errno.h>

#define SPI_OP   SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8) | SPI_LINES_SINGLE

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

const struct spi_dt_spec spidev_dt = SPI_DT_SPEC_GET(DT_NODELABEL(spidev), SPI_OP, 0);

/* Disk information structure required by Zephyr */
static struct disk_info nand_disk = {
    .name = "NAND_DISK",
    .ops = &nand_disk_ops,
    .dev = spidev_dt.bus
};

spi_nand_flash_config_t nand_flash_config = {
    .spi_dev = &spidev_dt,
};

spi_nand_flash_device_t *device_handle = NULL;


int nand_disk_access_init(struct disk_info *disk) {
    
    return spi_nand_flash_init_device(&nand_flash_config, &device_handle);
}

int nand_disk_access_status(struct disk_info *disk) {
    if (!device_is_ready(disk->dev)) {
        LOG_ERR("SPI device is not ready");
        return DISK_STATUS_UNINIT;
    }
    return DISK_STATUS_OK;
}

int nand_disk_access_read(struct disk_info *disk, uint8_t *data_buf, uint32_t start_sector, uint32_t num_sector) {
    LOG_DBG("nand_disk_access_read - disk=%s, start_sector=%u, num_sector=%u", disk->name, start_sector, num_sector);
    uint16_t sector_size;
    int ret = spi_nand_flash_get_sector_size(device_handle, &sector_size);
    if (ret < 0) {
        LOG_ERR("Error getting sector size: %d", ret);
        return -EIO;
    }

    for (uint32_t i = 0; i < num_sector; ++i) {
        ret = spi_nand_flash_read_sector(device_handle, data_buf + i * sector_size, start_sector + i);
        if (ret < 0) {
            LOG_ERR("Failed to read sector %d: %d", start_sector + i, ret);
            return -EIO;
        }
    }
    return 0;

}

int nand_disk_access_write(struct disk_info *disk, const uint8_t *data_buf, uint32_t start_sector, uint32_t num_sector) {
    LOG_DBG("nand_disk_access_write - disk=%s, start_sector=%u, num_sector=%u", disk->name, start_sector, num_sector);
    uint16_t sector_size;
    int ret = spi_nand_flash_get_sector_size(device_handle, &sector_size);
    if (ret < 0) {
        LOG_ERR("Error getting sector size: %d", ret);
        return -EIO;
    }

    for (uint32_t i = 0; i < num_sector; ++i) {
        ret = spi_nand_flash_write_sector(device_handle, data_buf + i * sector_size, start_sector + i);
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
            ret = spi_nand_flash_get_capacity(device_handle, (uint16_t *)buff);
            if (ret < 0) {
                LOG_ERR("Failed to get capacity: error %d", ret);
                return -EIO;
            }
            break;

        case DISK_IOCTL_GET_SECTOR_SIZE:
            // Assuming that the sector size function requires the device handle and a pointer to store the result
            ret = spi_nand_flash_get_sector_size(device_handle, (uint16_t *)buff);
            if (ret < 0) {
                LOG_ERR("Failed to get sector size: error %d", ret);
                return -EIO;
            }
            break;

        case DISK_IOCTL_CTRL_SYNC:
            // Assuming that the sync function only requires the device handle and returns status
            ret = spi_nand_flash_sync(device_handle);
            if (ret < 0) {
                LOG_ERR("Failed to sync device: error %d", ret);
                return -EIO;
            }
            break;

        default:
            LOG_ERR("Unsupported IOCTL command: %d", cmd);
            return -EINVAL;
    }
    return 0;
}


int disk_nand_init(void)
{
    LOG_INF("Initializing disk NAND flash");
    int ret = disk_access_register(&nand_disk);  // Register the disk
    if (ret) {
        LOG_ERR("Failed to register NAND disk");
        return ret;
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