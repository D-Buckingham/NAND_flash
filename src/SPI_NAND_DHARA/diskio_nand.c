//#include <fs/fs.h>
#include "nand_top_layer.h"
#include "zephyr/storage/disk_access.h"

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(diskio_nand, CONFIG_LOG_DEFAULT_LEVEL);

static int nand_disk_access_init(struct disk_info *disk);
static int nand_disk_access_status(struct disk_info *disk);
static int nand_disk_access_read(struct disk_info *disk, uint8_t *data_buf, uint32_t start_sector, uint32_t num_sector);
static int nand_disk_access_write(struct disk_info *disk, const uint8_t *data_buf, uint32_t start_sector, uint32_t num_sector);
static int nand_disk_access_ioctl(struct disk_info *disk, uint8_t cmd, void *buff);

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
    .device = spidev_dt -> bus;//spidev_dt.bus???
};



static int nand_disk_access_init(struct disk_info *disk) {

    spi_nand_flash_config_t nand_flash_config = {
        .spi_dev = &spidev_dt,
    };

    spi_nand_flash_device_t *device_handle = NULL;

    return spi_nand_flash_init_device(&nand_flash_config, &device_handle);//TODO correctly handled? &
}

static int nand_disk_access_status(struct disk_info *disk) {
    /* Implement the check to determine if the device is ready */
    return DISK_STATUS_OK;
}

static int nand_disk_access_read(struct disk_info *disk, uint8_t *data_buf, uint32_t start_sector, uint32_t num_sector) {
    return spi_nand_flash_read_sector(/* parameters */);
}

static int nand_disk_access_write(struct disk_info *disk, const uint8_t *data_buf, uint32_t start_sector, uint32_t num_sector) {
    return spi_nand_flash_write_sector(/* parameters */);
}

static int nand_disk_access_ioctl(struct disk_info *disk, uint8_t cmd, void *buff) {
    switch (cmd) {
        case DISK_IOCTL_GET_SECTOR_COUNT:
            return spi_nand_flash_get_capacity(/* parameters */);

        case DISK_IOCTL_GET_SECTOR_SIZE:
            return spi_nand_flash_get_sector_size(/* parameters */);

        case DISK_IOCTL_CTRL_SYNC:
            return spi_nand_flash_sync(/* parameters */);

        default:
            return -EINVAL;
    }
}