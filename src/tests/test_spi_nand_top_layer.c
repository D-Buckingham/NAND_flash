
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#include <stdlib.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zephyr/drivers/spi.h>
//#include <zephyr/ztest.h>
#include <assert.h>

#include    "test_spi_nand_top_layer.h"
#include    "nand_top_layer.h"

LOG_MODULE_REGISTER(test_spi_nand_top_layer, CONFIG_LOG_DEFAULT_LEVEL);


#define PATTERN_SEED    0x12345678

//Initialize the SPI device, TODO remove, since in the main we do this


//put the spi_handle into the spi_nand_flash_device_t struct and initialize the device
static void setup_nand_flash(spi_nand_flash_device_t **out_handle, const struct spi_dt_spec *spi_handle)
{

    spi_nand_flash_config_t nand_flash_config = {
        .spi_dev = spi_handle,
    };

    spi_nand_flash_device_t *device_handle = NULL;

    int ret = spi_nand_flash_init_device(&nand_flash_config, &device_handle);//TODO correctly handled? &
    if(ret != 0){
        LOG_ERR("Initialization of device on top layer, error: %d", ret);
    }else{
        LOG_INF("device on top layer initialized");
    }
    *out_handle = device_handle;
}



int test1_setup_erase_deinit_top_layer(const struct spi_dt_spec *spi)
{
    spi_nand_flash_device_t *nand_flash_device_handle = NULL;
    setup_nand_flash(&nand_flash_device_handle, spi);
    int err;
    err = spi_nand_erase_chip(nand_flash_device_handle);
    if(err != 0){
        LOG_ERR("Erase chip of device on top layer, error: %d", err);
        return -1;
    }
    err = spi_nand_flash_deinit_device(nand_flash_device_handle);
    if(err != 0){
        LOG_ERR("Deinitialize device on top layer, error: %d", err);
        return -1;
    }
    return 0;
    
}

static void check_buffer(uint32_t seed, const uint8_t *src, size_t count)
{
    srand(seed);
    for (size_t i = 0; i < count; ++i) {
        uint32_t val;
        memcpy(&val, src + i * sizeof(uint32_t), sizeof(val));
        //in case src buffer is not properly alligned, 
        // if ((uintptr_t)(src + i * sizeof(uint32_t)) % sizeof(uint32_t) != 0) {
        //     uint32_t temp;
        //     memcpy(&temp, src + i * sizeof(uint32_t), sizeof(temp));
        //     val = temp;
        // } else {
        //     val = *(uint32_t *)(src + i * sizeof(uint32_t));
        // }
        uint32_t expected = rand();  // Generate the next random number
        assert(val == expected);
        if (val != expected) {
            printf("Assertion failed at index %zu: Expected 0x%08X, got 0x%08X\n", i, expected, val);
            abort();  // Optionally abort to terminate the program
        }
    }
}

static void fill_buffer(uint32_t seed, uint8_t *dst, size_t count)
{
    srand(seed);
    for (size_t i = 0; i < count; ++i) {
        uint32_t val = rand();
        memcpy(dst + i * sizeof(uint32_t), &val, sizeof(val));//be careful because of misalignment
    }
}

static int do_single_write_test(spi_nand_flash_device_t *flash, uint32_t start_sec, uint16_t sec_count)
{
    uint8_t *temp_buf = NULL;
    uint8_t *pattern_buf = NULL;
    uint16_t sector_size, sector_num;

    int ret;

    ret = spi_nand_flash_get_capacity(flash, &sector_num);
    if(ret != 0){
        LOG_ERR("Unable to retrieve flash capacity, error: %d", ret);
        return -1;
    }

    ret = spi_nand_flash_get_sector_size(flash, &sector_size);
    if(ret != 0){
        LOG_ERR("Unable to get sector size, error: %d", ret);
        return -1;
    }

    if ((start_sec + sec_count) > sector_num) {
        LOG_ERR("Sector range exceeds flash size.");
        return -1;
    }

    pattern_buf = k_calloc(1, sector_size);//consider k_calloc
    if (!pattern_buf) {
        LOG_ERR("Failed to allocate pattern buffer");
        return -1;
    }
    temp_buf = k_calloc(1, sector_size);
    if (!temp_buf) {
        LOG_ERR("Failed to allocate temp buffer");
        k_free(pattern_buf);
        return -1;
    }

    fill_buffer(PATTERN_SEED, pattern_buf, sector_size / sizeof(uint32_t));

    for (int i = start_sec; i < sec_count; i++) {
        if(spi_nand_flash_write_sector(flash, pattern_buf, i) != 0){
            LOG_ERR("Failed to write sector at index %d", i);
            return -1;
        }
        memset((void *)temp_buf, 0x00, sector_size);
        if(spi_nand_flash_read_sector(flash, temp_buf, i) != 0){
            LOG_ERR("Failed to read sector at index %d", i);
            return -1;
        }
        check_buffer(PATTERN_SEED, temp_buf, sector_size / sizeof(uint32_t));
    }
    k_free(pattern_buf);
    k_free(temp_buf);
    return 0;
}

int test2_writing_tests_top_layer(const struct spi_dt_spec *spi)
{
    uint16_t sector_num, sector_size;
    spi_nand_flash_device_t *nand_flash_device_handle;
    setup_nand_flash(&nand_flash_device_handle, spi);

    if(spi_nand_flash_get_capacity(nand_flash_device_handle, &sector_num) != 0){
        LOG_ERR("Unable to retrieve flash capacity");
        return -1;
    }
    if(spi_nand_flash_get_sector_size(nand_flash_device_handle, &sector_size) != 0){
        LOG_ERR("Unable to get sector size");
        return -1;
    }
    printf("Number of sectors: %d, Sector size: %d\n", sector_num, sector_size);

    if(do_single_write_test(nand_flash_device_handle, 1, 16)!= 0){
        LOG_ERR("fails first single write test");
    }

    if (do_single_write_test(nand_flash_device_handle, 16, 32) != 0) {
        LOG_ERR("Failed second single write test");
    }

    if (do_single_write_test(nand_flash_device_handle, 32, 64) != 0) {
        LOG_ERR("Failed third single write test");
    }

    if (do_single_write_test(nand_flash_device_handle, 64, 128) != 0) {
        LOG_ERR("Failed fourth single write test");
    }

    if (do_single_write_test(nand_flash_device_handle, sector_num / 2, 32) != 0) {
        LOG_ERR("Failed fifth single write test at middle of the flash");
    }

    if (do_single_write_test(nand_flash_device_handle, sector_num / 2, 256) != 0) {
        LOG_ERR("Failed sixth single write test at middle with larger span");
    }

    if (do_single_write_test(nand_flash_device_handle, sector_num - 20, 16) != 0) {
        LOG_ERR("Failed last single write test near the end of the flash");
    }

    if(spi_nand_flash_deinit_device(nand_flash_device_handle) != 0){
        LOG_ERR("Deinitialize device on top layer");
        return -1;
    }
    return 0;
}

int test_nand_top_layer(const struct spi_dt_spec *spidev_dt){
    LOG_INF("Starting tests top layer");
    if(test1_setup_erase_deinit_top_layer(spidev_dt) != 0){
        LOG_ERR("Failed first test top layer above DHARA");
        return -1;
    }

    if(test2_writing_tests_top_layer(spidev_dt) != 0){
        LOG_ERR("Failed second test top layer above DHARA");
        return -1;
    }
    LOG_INF("Successful tests DHARA top layer");
    return 0;
}
