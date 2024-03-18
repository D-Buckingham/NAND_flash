#include "spi_nand_oper.h"
#include "spi_nand_oper_tests.h"

#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>                                                                                                                                                     
#include <zephyr/drivers/spi.h>
#include "nand.h"



LOG_MODULE_REGISTER(test_spi_nand_oper, CONFIG_LOG_DEFAULT_LEVEL);


int test_write_register_spi_nand(const struct device *dev){
     LOG_INF("Test 1: test write register");
     if (!device_is_ready(dev)) {
        LOG_ERR("Device not ready");
        return -1;
    }

    /**
     * Writing to a register and accidentally putting it in a lock just for testing 
     * is nonsenical
     * thus nothing implemented yet
    */
}



int test_read_page_spi_nand(const struct device *dev){
    LOG_INF("Test 2: test read page 1 to cache");
    if (!device_is_ready(dev)) {
        LOG_ERR("Device not ready");
        return -1;
    }

    int err;
    err = spi_nand_read_page(dev, 1); 
    if (err != 0) {
        LOG_ERR("Failed to read page 1 to cache %u, error: %d", page, err);
        return -1;
    }
    LOG_INF("Test 2: Successfull");
}



int test_read_cache_spi_nand(const struct device *dev){
    LOG_INF("Test 3: test read cache");
    if (!device_is_ready(dev)) {
        LOG_ERR("Device not ready");
        return -1;
    }

    //assuming data alread read into cash
    uint16_t test_buffer;
    int ret = spi_nand_read(dev, (uint8_t *)&test_buffer, 0, 2);
    if (ret != 0) {
        LOG_ERR("Failed to read into test buffer, err: %d", ret);
        return -1; // Assume page in cash
    }
    LOG_INF("Test 3: Successfull, data: %d", test_buffer);
}




int test_load_and_execute_program_spi_nand(const struct device *dev){
    LOG_INF("Test 4: test loading to cache and committing data to nand array");
    uint8_t data = 0xCC;
    uint16_t used_marker = 0;

    if (!device_is_ready(dev)) {
        LOG_ERR("Device not ready");
        return -1;
    }

    int ret = spi_nand_write_enable(dev);
    if (ret) {
        LOG_ERR("Failed to enable write, error: %d", ret);
        return -1;
    }

    ret = spi_nand_program_load(dev, &data, 0, 1);
    if (ret) {
        LOG_ERR("Failed to load program, error: %d", ret);
        return -1;
    }

    ret = spi_nand_program_load(dev, &used_marker, 1 + 2, 2);
    if (ret) {
        LOG_ERR("Failed to load used marker, error: %d", ret);
        return -1;
    }

    LOG_INF("Test 4: Successfull");
}



int test_erase_block_spi_nand(const struct device *dev){
    LOG_INF("Test 5: test erase block");
    uint8_t status;
    if (!device_is_ready(dev)) {
        LOG_ERR("Device not ready");
        return -1;
    }

    int ret = spi_nand_write_enable(dev);
    if (ret != 0) {
        LOG_ERR("Failed to enable write, error: %d", ret);
        return -1;
    }

    ret = spi_nand_erase_block(dev, 1);
    if (ret != 0) {
        LOG_ERR("Failed to erase page 1, error: %d", ret);
        return -1;
    }
    ret = wait_for_ready_nand(dev, 70, &status);
    if (ret != 0) {
        LOG_ERR("Failed to wait for ready, error: %d", ret);
        return -1;
    }
    if ((status & STAT_ERASE_FAILED) != 0) {
        LOG_ERR("Failed to erase page in block, test");
        return -1;
    }

}



int test_IDs_spi_nand(const struct device *dev){

    LOG_INF("Test 6: test getting device & manufacturer ID");

    if (!device_is_ready(dev)) {
        LOG_ERR("Device not ready");
        //return -ENODEV;
    }

    uint8_t device_id[2];
    int ret = spi_nand_device_id(dev, device_id);
    if (ret < 0) {
        LOG_ERR("Failed to read device ID");
    } else {
        LOG_INF("SPI NAND Device ID: 0x%x 0x%x", device_id[0], device_id[1]);
    }
    return ret;
}


int test_SPI_NAND_Communicator_all_tests(const struct device *dev){
    int ret = test_read_page_spi_nand(dev);
    if (ret )
}