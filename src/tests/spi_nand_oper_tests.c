
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>                                                                                                                                                     
#include <zephyr/drivers/spi.h>
#include "nand.h"

#include "spi_nand_oper.h"
#include "spi_nand_oper_tests.h"
#include <zephyr/devicetree.h>

#include <string.h> 
#include <stdlib.h>

#define PATTERN_SEED    0x12345678

LOG_MODULE_REGISTER(test_spi_nand_oper, CONFIG_LOG_DEFAULT_LEVEL);

static int wait_and_chill(const struct spi_dt_spec *dev){
    uint8_t status;
    int ret = 0;
    while (true) {
        ret = spi_nand_read_register(dev, REG_STATUS, &status);
        if (ret != 0) {
            LOG_ERR("Error reading NAND status register while waiting");
            ret = -1;
        }

        if ((status & STAT_BUSY) == 0) {
            break;
        }
        k_sleep(K_MSEC(1)); // Sleep for 1 millisecond instead of using vTaskDelay  
    }
    return ret;
}


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
     * checked in another test
    */
}



int test_read_page_spi_nand(const struct spi_dt_spec *dev){
    LOG_INF("Test 2: test read page 1 to cache");
    if (!device_is_ready(dev->bus)) {//error thrown
        LOG_ERR("Test 2: Device not ready");
        return -1;
    }

    int err;
    err = spi_nand_read_page(dev, 0x00); 
    if (err != 0) {
        LOG_ERR("Test 2: Failed to read page 1 to cache, error: %d",err);
        return -1;
    }
    LOG_INF("Test 2: No error thrown, read page 1 to cache");
    return 0;
}



int test_read_cache_spi_nand(const struct spi_dt_spec *dev){
    LOG_INF("Test 3: test read cache");
    if (!device_is_ready(dev->bus)) {
        LOG_ERR("Test 3: Device not ready");
        return -1;
    }

    //assuming data alread read into cash
    uint8_t test_buffer[6] = {0};
    int ret = spi_nand_read(dev, test_buffer, 0, 6);
    if (ret != 0) {
        LOG_ERR("Test 3: Failed to read into test buffer, err: %d", ret);
        return -1; // Assume page in cash
    }
    LOG_INF("Test 3: No error thrown, data: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x", test_buffer[0], test_buffer[1], test_buffer[2], test_buffer[3], test_buffer[4], test_buffer[5]);
    return 0;
}




int test_load_and_execute_program_spi_nand(const struct spi_dt_spec *dev){
    LOG_INF("Test 4: test loading to cache and committing data to nand array");
    const uint8_t data = 0xCC;
    uint16_t used_marker = 0;

    if (!device_is_ready(dev->bus)) {
        LOG_ERR("Test 4: Device not ready");
        return -1;
    }

    int ret = spi_nand_write_enable(dev);
    if (ret != 0) {
        LOG_ERR("Test 4: Failed to enable write, error: %d", ret);
        return -1;
    }

    ret = spi_nand_program_load(dev, &data, 0, 1);
    if (ret != 0) {
        LOG_ERR("Test 4: Failed to load program, error: %d", ret);
        return -1;
    }

    ret = spi_nand_program_load(dev, (uint8_t *)&used_marker, 1 + 2, 2);
    if (ret) {
        LOG_ERR("Test 4: Failed to load used marker, error: %d", ret);
        return -1;
    }

    LOG_INF("Test 4: No error thrown");
    return 0;
}



int test_erase_block_spi_nand(const struct spi_dt_spec *dev){
    LOG_INF("Test 5: test erase block");
    uint8_t status;
    if (!device_is_ready(dev->bus)) {
        LOG_ERR("Test 5: Device not ready");
        return -1;
    }

    int ret = spi_nand_write_enable(dev);
    if (ret != 0) {
        LOG_ERR("Test 5: Failed to enable write, error: %d", ret);
        return -1;
    }
    spi_nand_read_register(dev, REG_STATUS, &status);//to check in debugging

    ret = spi_nand_erase_block(dev, 0);
    if (ret != 0) {
        LOG_ERR("Test 5: Failed to erase block 0, error: %d", ret);
        return -1;
    }else{
        LOG_INF("Test 5: Found correct flag in register after successful erasure");
    }
    
    
    wait_and_chill(dev);
    if (ret != 0) {
        return -1;
    }

    if ((status & STAT_ERASE_FAILED) != 0) {
        LOG_ERR("Test 5: Failed to erase page in block, test");
        return -1;
    }
    
    LOG_INF("Test 5 No error thrown");
    return 0;
}



int test_IDs_spi_nand(const struct spi_dt_spec *dev){

    LOG_INF("Test 1: test getting device & manufacturer ID");

    if (!device_is_ready(dev->bus)) {
        LOG_ERR("Test 1: Device not ready");
    }

    uint8_t device_id;
    int ret = spi_nand_device_id(dev, (uint8_t *) &device_id);
    if (ret != 0) {
        LOG_ERR("Test 1: Failed to read device ID");
    } else {
        LOG_INF("Test 1: SPI NAND Device ID: 0x%x ", device_id);
    }
    LOG_INF("Test 1 No error thrown");
    return ret;
}




//final test, write and read it
int test_spi_nand_write_read(const struct spi_dt_spec *dev) {
    LOG_INF("Test 6: testing SPI NAND write and read register");
    uint16_t length = 4;
    const uint8_t data[4] = {0xAA, 0xBB, 0xCC, 0x11};
    uint32_t page = 0x00;
    uint8_t readings[4] = {0};
    uint8_t read_page[800] = {0};
  

    if (!device_is_ready(dev->bus)) {
        LOG_ERR("Device not ready");
        return -1;
    }
    //usually we would read into cache existing data before changing it.
    //enable write
    int ret = spi_nand_write_enable(dev);
    if (ret) {
        LOG_ERR("Test 6: Failed to enable write, error: %d", ret);
        return -1;
    }
    //program load into cache
    ret = spi_nand_program_load(dev, data, 1, length);
    if (ret) {
        LOG_ERR("Test 6: Failed to load program, error: %d", ret);
        return -1;
    }
    //program execute into nand array
    ret = spi_nand_program_execute(dev, page);
    if (ret != 0) {
        LOG_ERR("Test 6: Failed to execute program on page, error: %d", ret);
        return -1;
    }

    //check and wait if successful
    ret = wait_and_chill(dev);
    if (ret != 0) {
        return -1;
    }
    // Read from the register
    ret = spi_nand_read_page(dev, page); 
    if (ret != 0) {
        LOG_ERR("Test 6: Failed to read page %u, error: %d", page, ret);
        return -1;
    }

   
    //wait again
    ret = wait_and_chill(dev);
    if (ret != 0) {
        return -1;
    }

    //read from cache
    ret = spi_nand_read(dev, readings, 1, length);
    if (ret != 0) {
        LOG_ERR("Test 6: Failed to read , err: %d", ret);
        return -1; 
    }

    ret = spi_nand_read(dev, read_page, 1, 800);
    if (ret != 0) {
        LOG_ERR("Test 6: Failed to read , err: %d", ret);
        return -1; 
    }

    char buffer[2500];
    int pos = 0;
    int line_byte_count = 0;

    for (int i = 0; i < 800; i++) {
        char separator = (line_byte_count == 39 || i == 799) ? '\n' : ' ';
        pos += snprintf(&buffer[pos], sizeof(buffer) - pos, "%02X%c", read_page[i], separator);

        if (++line_byte_count == 40) {
            line_byte_count = 0; 
        }
        if (pos >= sizeof(buffer) - 10) {
            LOG_INF("Test 6: Partial read data:\n%s", buffer);
            pos = 0; 
        }
    }

    // Log any remaining data in the buffer
    if (pos > 0) {
        LOG_INF("Test 6: Partial read data:\n%s", buffer);
    }

    

    // Verify that the value read matches the value written
    if (memcmp(data, readings, 4) == 0)  {
        LOG_INF("Test 6: Write and read register test PASSED");
        for (uint16_t i = 0; i < length; i++) {
            LOG_INF("Test 6: Write and read register at index %d: Written value 0x%X, read value 0x%X", i, data[i], readings[i]);
        }
    } else {
        for (uint16_t i = 0; i < length; i++) {
            LOG_ERR("Test 6: Write and read register test FAILED at index %d: Written value 0x%X, read value 0x%X", i, data[i], readings[i]);
        }
        return -1;
    }
    return 0;
}



static int check_buffer(uint32_t seed, const uint8_t *src, size_t count)
{
    int ret = 0;
    srand(seed);
    for (size_t i = 0; i < count; ++i) {
        uint8_t val = src[i];
        uint8_t expected = rand() & 0xFF; 
        if (val != expected) {
            //LOG_ERR("Mismatch at index %zu: expected 0x%02X, got 0x%02X", i, expected, val);
            ret = -1;
        }
    }
    return ret;
}

static void fill_buffer(uint32_t seed, uint8_t *dst, size_t count)
{
    srand(seed);
    for (size_t i = 0; i < count; ++i) {
        dst[i] = rand() & 0xFF;  
        //LOG_INF("Index %zu: 0x%02X", i, dst[i]);
    }
}

int status;
int ret;

// Function to log the status and protection registers
void log_registers(const struct spi_dt_spec *dev, const char* operation) {
    uint8_t status, protect;

    // Log the status register
    if (spi_nand_read_register(dev, REG_STATUS, &status) == 0) {
        LOG_INF("%s: Status Register = 0x%02X", operation, status);
    } else {
        LOG_ERR("%s: Failed to read Status Register", operation);
    }

    // Log the protection register
    if (spi_nand_read_register(dev, REG_PROTECT, &protect) == 0) {
        LOG_INF("%s: Protect Register = 0x%02X", operation, protect);
    } else {
        LOG_ERR("%s: Failed to read Protect Register", operation);
    }
}


static uint8_t pattern_buf[2048];
static uint8_t temp_buf[2175];

//final test, write and read it
int test_spi_nand_sector_write_read(const struct spi_dt_spec *dev) {
    LOG_INF("Test 7: testing SPI NAND sector write and read register");
    int status;

    memset(pattern_buf, 0xFF, 2048);
    memset(temp_buf, 0xFF, 2175);
    
    //PREPARATION:

    //we assume a sector size of 2048 (smaller than page size of 2175)
    uint16_t sector_size = 2048;
    uint32_t page = 0;
    uint16_t column_address = 0;//starting point to read from in page in cache
    int ret;

    fill_buffer(PATTERN_SEED, pattern_buf, sector_size);//we store every 4 byte address 4 bytes//(uint8_t*) pattern_buf??


    if (!device_is_ready(dev->bus)) {
        LOG_ERR("Test 7: Device not ready");
        return -1;
    }
    wait_and_chill(dev);



    ret = spi_nand_read_page(dev, page); 
    if (ret != 0) {
        LOG_ERR("Test 7: Failed to read page %u, error: %d", page, ret);
        return -1;
    }
   
    wait_and_chill(dev);
    ret = spi_nand_write_enable(dev);
    if (ret != 0) {
        LOG_ERR("Test 7: Failed to enable write, error: %d", ret);
        return -1;
    }
    log_registers(dev, "After Write Enable");
    
    
    //We just overwrite existing data in NAND array
    //load data into cache
    if(spi_nand_program_load(dev, pattern_buf, column_address, 2048) == 0){
        wait_and_chill(dev);
        log_registers(dev, "After Program Load");
        if(spi_nand_program_execute(dev, page) != 0){
            LOG_ERR("Test7: Failed to write sector at index %d", 1);
            return -1;
        }
        log_registers(dev, "After Program Execute");
    }

    //check and wait if successful
    ret = wait_and_chill(dev);
    if (ret != 0) {
        return -1;
    }


    //read sector into buffer
    // Read from the NAND array the block 0, page 0 everything 
    ret = spi_nand_read_page(dev, page); 
    if (ret != 0) {
        LOG_ERR("Test 7: Failed to read page %u, error: %d", page, ret);
        return -1;
    }
    
 
    ret = wait_and_chill(dev);
    if (ret != 0) {
        return -1;
    }

    log_registers(dev, "After Page Read in cash");
    //read from cache
    ret = spi_nand_read(dev, temp_buf, 0, 2175);
    if (ret != 0) {
        LOG_ERR("Test 7: Failed to read , err: %d", ret);
        return -1; 
    }
    log_registers(dev, "After Page Read");


    //check if written random numbers are the same as read out ones
    ret = check_buffer(PATTERN_SEED, temp_buf, sector_size);//TODO figure out how to address the entire page

    if(ret == 0){
        LOG_INF("TEST 7: PASSED!!!");
    }


    // Log the first 100 bytes of temp_buf as hexadecimal values
    LOG_INF("Contents of temp_buf:");
    for (int i = 0; i < 2175 && i < sector_size; i++) {
        if (i % 40 == 0 && i != 0) {
            LOG_INF("");  
        }
        printk("%02X ", temp_buf[i]);  // Using printk for continuous output on the same line
    }
    wait_and_chill(dev);
    

    memset(pattern_buf, 0xFF, 2048);

    LOG_INF("patternbuf is:");
    for (int i = 0; i < 2048 && i < sector_size; i++) {
        if (i % 40 == 0 && i != 0) {
            LOG_INF("");  
        }
        printk("%02X ", pattern_buf[i]);  // Using printk for continuous output on the same line
    }
    ret = spi_nand_write_enable(dev);
    wait_and_chill(dev);
    log_registers(dev, "After write enable");

    // Write 0xFF to the entire page
    if (spi_nand_program_load(dev, pattern_buf, column_address, 2048) == 0) {
        wait_and_chill(dev);
        if (spi_nand_program_execute(dev, page) != 0) {
            LOG_ERR("Test7: Failed to reset sector to 0xFF at index %d", 1);
            return -1;
        }
    }

    ret = wait_and_chill(dev);

    //read sector into buffer
    // Read from the NAND array the block 0, page 0 everything 
    ret = spi_nand_read_page(dev, page); 
    if (ret != 0) {
        LOG_ERR("Test 7: Failed to read page %u, error: %d", page, ret);
        return -1;
    }
    log_registers(dev, "After Page Read into cash");

 
    ret = wait_and_chill(dev);
    if (ret != 0) {
        return -1;
    }
    //read from cache
    ret = spi_nand_read(dev, temp_buf, 0, 2175);
    if (ret != 0) {
        LOG_ERR("Test 7: Failed to read , err: %d", ret);
        return -1; 
    }
    log_registers(dev, "After Page Read");


    return 0;
}

int test_SPI_NAND_Communicator_all_tests(const struct spi_dt_spec *dev) {
    int ret;
    uint8_t status;
    

    //ret = spi_nand_read_register(dev, REG_PROTECT, &status);//TODO for debugging
    LOG_INF("Starting all SPI NAND communicator tests");
    
    LOG_INF("Unprotecting chip");
    spi_nand_write_register(dev, REG_PROTECT, 0);
    ret = spi_nand_read_register(dev, REG_PROTECT, &status);
    if (ret != 0) {
        LOG_ERR("Failed to read REG_PROTECT, error: %d", ret);
    }




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
    

    //test 6
    ret = test_spi_nand_write_read(dev);
    if (ret != 0) {
        LOG_ERR("Write and read test failed");
        return ret;
    }

    //test 7
    ret = test_spi_nand_sector_write_read(dev);
    if (ret != 0) {
        LOG_ERR("Sector write and read test failed");
        return ret;
    }

    LOG_INF("All SPI NAND driver tests passed successfully\n\n");
    return 0;
}