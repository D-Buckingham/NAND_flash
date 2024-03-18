#include "spi_nand_oper.h"

#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>                                                                                                                                                     
#include <zephyr/drivers/spi.h>



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
    LOG_INF("Test 2: test read page 1 to cash");
    if (!device_is_ready(dev)) {
        LOG_ERR("Device not ready");
        return -1;
    }

    int err;
    err = spi_nand_read_page(dev, 1); 
    if (err != 0) {
        LOG_ERR("Failed to read page 1 to cash %u, error: %d", page, err);
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
int test_execute_program_spi_nand(const struct device *dev){
    LOG_INF("Test 4: test execution program");
    if (!device_is_ready(dev)) {
        LOG_ERR("Device not ready");
        return -1;
    }
}
int test_erase_block_spi_nand(const struct device *dev){
    LOG_INF("Test 5: test erase block");
    if (!device_is_ready(dev)) {
        LOG_ERR("Device not ready");
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