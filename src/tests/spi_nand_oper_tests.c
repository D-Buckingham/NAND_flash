
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>                                                                                                                                                     
#include <zephyr/drivers/spi.h>
#include "nand.h"

#include "spi_nand_oper.h"
#include "spi_nand_oper_tests.h"
#include <zephyr/devicetree.h>

#include <string.h> 

LOG_MODULE_REGISTER(test_spi_nand_oper, CONFIG_LOG_DEFAULT_LEVEL);


int test_write_register_spi_nand(const struct spi_dt_spec *dev){
     LOG_INF("Test ?: test write register");
     if (!device_is_ready(dev->bus)) {
        LOG_ERR("Device not ready");
        return -1;
    }
    return 0;

    /**
     * Writing to a register and accidentally putting it in a lock just for testing 
     * is nonsenical
     * thus nothing implemented yet
    */
}



int test_read_page_spi_nand(const struct spi_dt_spec *dev){
    LOG_INF("Test 2: test read page 1 to cache");
    if (!device_is_ready(dev->bus)) {//error thrown
        LOG_ERR("Device not ready");
        return -1;
    }

    int err;
    err = spi_nand_read_page(dev, 0x00); 
    if (err != 0) {
        LOG_ERR("Failed to read page 1 to cache, error: %d",err);
        return -1;
    }
    LOG_INF("Test 2: No error thrown, read page 1 to cache");
    return 0;
}



int test_read_cache_spi_nand(const struct spi_dt_spec *dev){
    LOG_INF("Test 3: test read cache");
    if (!device_is_ready(dev->bus)) {
        LOG_ERR("Device not ready");
        return -1;
    }

    //assuming data alread read into cash
    uint8_t test_buffer[2] = {0};
    int ret = spi_nand_read(dev, test_buffer, 0, 2);
    if (ret != 0) {
        LOG_ERR("Failed to read into test buffer, err: %d", ret);
        return -1; // Assume page in cash
    }
    LOG_INF("Test 3: No error thrown, data: 0x%x 0x%x", test_buffer[0], test_buffer[1]);
    return 0;
}




int test_load_and_execute_program_spi_nand(const struct spi_dt_spec *dev){
    LOG_INF("Test 4: test loading to cache and committing data to nand array");
    uint8_t data = 0xCC;
    uint16_t used_marker = 0;

    if (!device_is_ready(dev->bus)) {
        LOG_ERR("Device not ready");
        return -1;
    }

    int ret = spi_nand_write_enable(dev);
    if (ret != 0) {
        LOG_ERR("Failed to enable write, error: %d", ret);
        return -1;
    }

    ret = spi_nand_program_load(dev, &data, 0, 1);
    if (ret != 0) {
        LOG_ERR("Failed to load program, error: %d", ret);
        return -1;
    }

    ret = spi_nand_program_load(dev, (uint8_t *)&used_marker, 1 + 2, 2);
    if (ret) {
        LOG_ERR("Failed to load used marker, error: %d", ret);
        return -1;
    }

    LOG_INF("Test 4: No error thrown");
    return 0;
}



int test_erase_block_spi_nand(const struct spi_dt_spec *dev){
    LOG_INF("Test 5: test erase block");
    uint8_t status;
    if (!device_is_ready(dev->bus)) {
        LOG_ERR("Device not ready");
        return -1;
    }

    int ret = spi_nand_write_enable(dev);
    if (ret != 0) {
        LOG_ERR("Failed to enable write, error: %d", ret);
        return -1;
    }
    spi_nand_read_register(dev, REG_STATUS, &status);//to check in debugging

    ret = spi_nand_erase_block(dev, 0);
    if (ret != 0) {
        LOG_ERR("Failed to erase block 0, error: %d", ret);
        return -1;
    }else{
        LOG_INF("Found correct flag in register after successful erasure");
    }
    
    
    // //check and wait if successful
    while (true) {
        
        int err = spi_nand_read_register(dev, REG_STATUS, &status);
        if (err != 0) {
            LOG_ERR("Error reading NAND status register");
        }

        if ((status & STAT_BUSY) == 0) {
            break;
        }     
        k_msleep(1);   
    }

    if ((status & STAT_ERASE_FAILED) != 0) {
        LOG_ERR("Failed to erase page in block, test");
        return -1;
    }
    
    LOG_INF("Test 5 No error thrown");
    return 0;
}



int test_IDs_spi_nand(const struct spi_dt_spec *dev){

    LOG_INF("Test 1: test getting device & manufacturer ID");

    if (!device_is_ready(dev->bus)) {
        LOG_ERR("Device not ready");
    }

    uint8_t device_id;
    int ret = spi_nand_device_id(dev, (uint8_t *) &device_id);
    if (ret != 0) {
        LOG_ERR("Failed to read device ID");
    } else {
        LOG_INF("SPI NAND Device ID: 0x%x ", device_id);
    }
    LOG_INF("Test 1 No error thrown");
    return ret;
}



//final test, write and read it
int test_spi_nand_write_read(const struct spi_dt_spec *dev) {
    LOG_INF("Test 6: testing SPI NAND write and read register");
    uint16_t length = 4;
    uint8_t data[4] = {0xAA, 0xBB, 0xCC, 0x11};
    uint32_t page = 0x00;
    uint8_t readings[4] = {0};

    

    if (!device_is_ready(dev->bus)) {
        LOG_ERR("Device not ready");
        return -1;
    }
    //enable write
    int ret = spi_nand_write_enable(dev);
    if (ret) {
        LOG_ERR("Failed to enable write, error: %d", ret);
        return -1;
    }
    //program load into cache
    ret = spi_nand_program_load(dev, data, 1, length);
    if (ret) {
        LOG_ERR("Failed to load program, error: %d", ret);
        return -1;
    }
    //program execute into nand array
    ret = spi_nand_program_execute(dev, page);
    if (ret != 0) {
        LOG_ERR("Failed to execute program on page, error: %d", ret);
        return -1;
    }

    //check and wait if successful
    while (true) {
        uint8_t status;
        int err = spi_nand_read_register(dev, REG_STATUS, &status);
        if (err != 0) {
            LOG_ERR("Error reading NAND status register");
        }

        if ((status & STAT_BUSY) == 0) {
            break;
        }
        k_sleep(K_MSEC(1)); // Sleep for 1 millisecond instead of using vTaskDelay
        
    }



    // Read from the register
    int err;
    err = spi_nand_read_page(dev, page); 
    if (err != 0) {
        LOG_ERR("Failed to read page %u, error: %d", page, err);
        return -1;
    }


    //wait again
    while (true) {
        uint8_t status;
        int err = spi_nand_read_register(dev, REG_STATUS, &status);
        if (err != 0) {
            LOG_ERR("Error reading NAND status register");
        }

        if ((status & STAT_BUSY) == 0) {
            break;
        }
        k_sleep(K_MSEC(1)); // Sleep for 1 millisecond instead of using vTaskDelay
        
    }

    //read from cache
    ret = spi_nand_read(dev, readings, 1, length);
    if (ret != 0) {
        LOG_ERR("Failed to read , err: %d", ret);
        return -1; 
    }

    

    // Verify that the value read matches the value written
    if (memcmp(data, readings, 4) == 0)  {
        LOG_INF("Write and read register test PASSED");
        for (uint16_t i = 0; i < length; i++) {
            LOG_INF("Write and read register at index %d: Written value 0x%X, read value 0x%X", i, data[i], readings[i]);
        }
    } else {
        for (uint16_t i = 0; i < length; i++) {
            LOG_ERR("Write and read register test FAILED at index %d: Written value 0x%X, read value 0x%X", i, data[i], readings[i]);
        }
        return -1;
    }
    return 0;
}




int test_SPI_NAND_Communicator_all_tests(const struct spi_dt_spec *dev) {
    int ret;
    
    LOG_INF("Starting all SPI NAND communicator tests");
    
    
    //test 1
    ret = test_IDs_spi_nand(dev);
    if (ret != 0) {
        LOG_ERR("Device & Manufacturer ID test failed");
        return ret;
    }
    
    

   //test 2
    ret = test_read_page_spi_nand(dev);
    if (ret != 0) {
        LOG_ERR("Read page to cache test failed");
        return ret;
    }
    
    
    //test 3
    ret = test_read_cache_spi_nand(dev);
    if (ret != 0) {
        LOG_ERR("Read cache test failed");
        return ret;
    }
    
    
    //test 4
    ret = test_load_and_execute_program_spi_nand(dev);
    if (ret != 0) {
        LOG_ERR("Load and execute program test failed");
        return ret;
    }
   

    //test 5
    ret = test_erase_block_spi_nand(dev);
    if (ret != 0) {
        LOG_ERR("Erase block test failed");
        return ret;
    }
    

    //test 5
    ret = test_spi_nand_write_read(dev);
    if (ret != 0) {
        LOG_ERR("write and read test failed");
        return ret;
    }

    LOG_INF("All SPI NAND communicator tests passed successfully");
    return 0;
}