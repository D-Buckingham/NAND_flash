//This file has to be renamed and go into the dhara folder

//#include "dhara/nand.h"



#include "spi_nand_oper.h"
#include "nand_top_layer.h"
#include "nand_top_layer.h"

#include <zephyr/drivers/spi.h>
#include <zephyr/device.h>
#include <zephyr/fs/fs.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>                                                                                                                                                     
#include <zephyr/drivers/spi.h>
#include "map.h"



#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

LOG_MODULE_REGISTER(dhara_glue, CONFIG_LOG_DEFAULT_LEVEL);

#define ROM_WAIT_THRESHOLD_US 1000

/*
esp_err_t wait_for_ready(spi_device_handle_t device, uint32_t expected_operation_time_us, uint8_t *status_out)
{
    if (expected_operation_time_us < ROM_WAIT_THRESHOLD_US) {
        esp_rom_delay_us(expected_operation_time_us);
    }

    while (true) {
        uint8_t status;
        ESP_RETURN_ON_ERROR(spi_nand_read_register(device, REG_STATUS, &status), TAG, "");

        if ((status & STAT_BUSY) == 0) {
            if (status_out) {
                *status_out = status;
            }
            break;
        }

        if (expected_operation_time_us >= ROM_WAIT_THRESHOLD_US) {
            vTaskDelay(1);
        }
    }

    return ESP_OK;
}*/

int wait_for_ready(const struct device *device, uint32_t expected_operation_time_us, uint8_t *status_out)
{
    // Assuming ROM_WAIT_THRESHOLD_US is defined somewhere globally
    if (expected_operation_time_us < ROM_WAIT_THRESHOLD_US) {
        k_busy_wait(expected_operation_time_us); // busy wait for microseconds
    }

    while (true) {
        uint8_t status;
        int err = spi_nand_read_register(device, REG_STATUS, &status);
        if (err) {
            LOG_ERR("Error reading NAND status register");
            return err; // Return error code directly
        }

        if ((status & STAT_BUSY) == 0) {
            if (status_out) {
                *status_out = status;
            }
            break;
        }

        if (expected_operation_time_us >= ROM_WAIT_THRESHOLD_US) {
            k_sleep(K_MSEC(1)); // Sleep for 1 millisecond instead of using vTaskDelay
        }
    }

    return 0; // Success
}



/*
static esp_err_t read_page_and_wait(spi_nand_flash_device_t *dev, uint32_t page, uint8_t *status_out)
{
    ESP_RETURN_ON_ERROR(spi_nand_read_page(dev->config.device_handle, page), TAG, "");

    return wait_for_ready(dev->config.device_handle, dev->read_page_delay_us, status_out);
}

static esp_err_t program_execute_and_wait(spi_nand_flash_device_t *dev, uint32_t page, uint8_t *status_out)
{
    ESP_RETURN_ON_ERROR(spi_nand_program_execute(dev->config.device_handle, page), TAG, "");

    return wait_for_ready(dev->config.device_handle, dev->program_page_delay_us, status_out);
}
*/
static int read_page_and_wait(struct spi_nand_flash_device_t *device, uint32_t page, uint8_t *status_out)
{
    int err;
    err = spi_nand_read_page(device -> config.spi_dev, page); 
    if (err) {
        LOG_ERR("Failed to read page %u", page);
        return err;
    }

    return wait_for_ready(device -> config.spi_dev,device -> read_page_delay_us, status_out);
}

static int program_execute_and_wait(struct spi_nand_flash_device_t *device, uint32_t page, uint8_t *status_out)
{
    int err;

    err = spi_nand_program_execute(device -> config.spi_dev, page);
    if (err) {
        LOG_ERR("Failed to execute program on page %u", page);
        return err;
    }

    return wait_for_ready(device -> config.spi_dev, device -> program_page_delay_us, status_out);
}




/*
int dhara_nand_is_bad(const struct dhara_nand *n, dhara_block_t b)
{
    spi_nand_flash_device_t *dev = __containerof(n, spi_nand_flash_device_t, dhara_nand);

    dhara_page_t first_block_page = b * (1 << n->log2_ppb);
    uint16_t bad_block_indicator;
    esp_err_t ret;

    ESP_GOTO_ON_ERROR(read_page_and_wait(dev, first_block_page, NULL), fail, TAG, "");

    // Read the first 2 bytes on the OOB of the first page in the block. This should be 0xFFFF for a good block
    ESP_GOTO_ON_ERROR(spi_nand_read(dev->config.device_handle, (uint8_t *) &bad_block_indicator, dev->page_size, 2),
                      fail, TAG, "");

    ESP_LOGD(TAG, "is_bad, block=%"PRIu32", page=%"PRIu32",indicator = %04x", b, first_block_page, bad_block_indicator);
    return bad_block_indicator != 0xFFFF;

fail:
    ESP_LOGE(TAG, "Error in dhara_nand_is_bad %d", ret);
    return 1;
}
*/
int dhara_nand_is_bad(const struct dhara_nand *n, dhara_block_t b)
{
    //const struct device *dev = DEVICE_DT_GET(DT_NODELABEL(arduino_spi));

    /**
     * to retrieve a pointer to a container structure given a pointer to a member of that structure. It's used to obtain
     * a pointer to a parent or enclosing structure by knowing the pointer to a member field and the type of the parent 
     * structure.
    */
    spi_nand_flash_device_t *dev = CONTAINER_OF(n, spi_nand_flash_device_t, dhara_nand);//struct necessary?

    dhara_page_t first_block_page = b * (1 << n->log2_ppb);
    uint16_t bad_block_indicator;
    int ret;

    ret = read_page_and_wait(dev, first_block_page, NULL);
    if (ret != 0) {
        LOG_ERR("Error reading page: %d", ret);
        return 1; // Assume bad block on error
    }

    // Assuming spi_nand_read function reads 'len' bytes starting from 'offset' to 'buf'
    // And assuming the OOB data can be accessed directly following the main data area
    ret = spi_nand_read(dev->config.spi_dev, (uint8_t *)&bad_block_indicator, dev->page_size, 2);
    if (ret != 0) {
        LOG_ERR("Failed to read bad block indicator, err: %d", ret);
        return 1; // Assume bad block on error
    }

    LOG_DBG("Block=%u, Page=%u, Indicator=%04x", b, first_block_page, bad_block_indicator);

    return bad_block_indicator != 0xFFFF;
}



/*
void dhara_nand_mark_bad(const struct dhara_nand *n, dhara_block_t b)
{
    spi_nand_flash_device_t *dev = __containerof(n, spi_nand_flash_device_t, dhara_nand);
    esp_err_t ret;

    dhara_page_t first_block_page = b * (1 << n->log2_ppb);
    uint16_t bad_block_indicator = 0;
    ESP_LOGD(TAG, "mark_bad, block=%"PRIu32", page=%"PRIu32",indicator = %04x", b, first_block_page, bad_block_indicator);

    ESP_GOTO_ON_ERROR(spi_nand_write_enable(dev->config.device_handle), fail, TAG, "");
    ESP_GOTO_ON_ERROR(spi_nand_erase_block(dev->config.device_handle, first_block_page),
                      fail, TAG, "");

    ESP_GOTO_ON_ERROR(spi_nand_write_enable(dev->config.device_handle), fail, TAG, "");
    ESP_GOTO_ON_ERROR(spi_nand_program_load(dev->config.device_handle, (const uint8_t *) &bad_block_indicator,
                                            dev->page_size, 2),
                      fail, TAG, "");
    ESP_GOTO_ON_ERROR(program_execute_and_wait(dev, first_block_page, NULL), fail, TAG, "");
    return;
fail:
    ESP_LOGE(TAG, "Error in dhara_nand_mark_bad %d", ret);
}
*/
//TODO check in datasheet process
void dhara_nand_mark_bad(const struct dhara_nand *n, dhara_block_t b)
{
    spi_nand_flash_device_t *dev = CONTAINER_OF(n, spi_nand_flash_device_t, dhara_nand);//struct?
    int ret;

    dhara_page_t first_block_page = b * (1 << n->log2_ppb);
    uint16_t bad_block_indicator = 0;
    LOG_DBG("mark_bad, block=%u, page=%u, indicator = %04x", b, first_block_page, bad_block_indicator);

    ret = spi_nand_write_enable(dev->config.spi_dev);
    if (ret) {
        LOG_ERR("Failed to enable write, error: %d", ret);
        return;
    }

    ret = spi_nand_erase_block(dev->config.spi_dev, first_block_page);
    if (ret) {
        LOG_ERR("Failed to erase block, error: %d", ret);
        return;
    }

    ret = spi_nand_write_enable(dev->config.spi_dev);
    if (ret) {
        LOG_ERR("Failed to enable write, error: %d", ret);
        return;
    }

    ret = spi_nand_program_load(dev->config.spi_dev, (const uint8_t *)&bad_block_indicator, dev->page_size, 2);
    if (ret) {
        LOG_ERR("Failed to program load, error: %d", ret);
        return;
    }

    ret = program_execute_and_wait(dev, first_block_page, NULL);
    if (ret) {
        LOG_ERR("Failed to execute program and wait, error: %d", ret);
        return;
    }
}



/*
int dhara_nand_erase(const struct dhara_nand *n, dhara_block_t b, dhara_error_t *err)
{
    ESP_LOGD(TAG, "erase_block, block=%"PRIu32",", b);
    spi_nand_flash_device_t *dev = __containerof(n, spi_nand_flash_device_t, dhara_nand);
    esp_err_t ret;

    dhara_page_t first_block_page = b * (1 << n->log2_ppb);
    uint8_t status;

    ESP_GOTO_ON_ERROR(spi_nand_write_enable(dev->config.device_handle), fail, TAG, "");
    ESP_GOTO_ON_ERROR(spi_nand_erase_block(dev->config.device_handle, first_block_page),
                      fail, TAG, "");
    ESP_GOTO_ON_ERROR(wait_for_ready(dev->config.device_handle,
                                     dev->erase_block_delay_us, &status),
                      fail, TAG, "");

    if ((status & STAT_ERASE_FAILED) != 0) {
        dhara_set_error(err, DHARA_E_BAD_BLOCK);
        return -1;
    }

    return 0;

fail:
    ESP_LOGE(TAG, "Error in dhara_nand_erase %d", ret);
    return -1;
}
*/
int dhara_nand_erase(const struct dhara_nand *n, dhara_block_t b, dhara_error_t *err)
{
    LOG_DBG("erase_block, block=%u", b);
    struct spi_nand_flash_device_t *dev = CONTAINER_OF(n, struct spi_nand_flash_device_t, dhara_nand);
    int ret;

    dhara_page_t first_block_page = b * (1 << n->log2_ppb);
    uint8_t status;

    ret = spi_nand_write_enable(dev->config.spi_dev);
    if (ret) {
        LOG_ERR("Failed to enable write, error: %d", ret);
        return -1;
    }

    ret = spi_nand_erase_block(dev->config.spi_dev, first_block_page);
    if (ret) {
        LOG_ERR("Failed to erase block, error: %d", ret);
        return -1;
    }

    ret = wait_for_ready(dev->config.spi_dev, dev->erase_block_delay_us, &status);
    if (ret) {
        LOG_ERR("Failed to wait for ready, error: %d", ret);
        return -1;
    }

    if ((status & STAT_ERASE_FAILED) != 0) {
        dhara_set_error(err, DHARA_E_BAD_BLOCK);
        return -1;
    }

    return 0;
}





/*
int dhara_nand_prog(const struct dhara_nand *n, dhara_page_t p, const uint8_t *data, dhara_error_t *err)
{
    ESP_LOGV(TAG, "prog, page=%"PRIu32",", p);
    spi_nand_flash_device_t *dev = __containerof(n, spi_nand_flash_device_t, dhara_nand);
    esp_err_t ret;
    uint8_t status;
    uint16_t used_marker = 0;

    ESP_GOTO_ON_ERROR(read_page_and_wait(dev, p, NULL), fail, TAG, "");
    ESP_GOTO_ON_ERROR(spi_nand_write_enable(dev->config.device_handle), fail, TAG, "");
    ESP_GOTO_ON_ERROR(spi_nand_program_load(dev->config.device_handle, data, 0, dev->page_size),
                      fail, TAG, "");
    ESP_GOTO_ON_ERROR(spi_nand_program_load(dev->config.device_handle, (uint8_t *)&used_marker,
                                            dev->page_size + 2, 2),
                      fail, TAG, "");
    ESP_GOTO_ON_ERROR(program_execute_and_wait(dev, p, &status), fail, TAG, "");

    if ((status & STAT_PROGRAM_FAILED) != 0) {
        ESP_LOGD(TAG, "prog failed, page=%"PRIu32",", p);
        dhara_set_error(err, DHARA_E_BAD_BLOCK);
        return -1;
    }

    return 0;
fail:
    ESP_LOGE(TAG, "Error in dhara_nand_prog %d", ret);
    return -1;
}
*/
int dhara_nand_prog(const struct dhara_nand *n, dhara_page_t p, const uint8_t *data, dhara_error_t *err)
{
    LOG_DBG("prog, page=%u", p);
    spi_nand_flash_device_t *dev = CONTAINER_OF(n, spi_nand_flash_device_t, dhara_nand);
    int ret;
    uint8_t status;
    uint16_t used_marker = 0;

    ret = read_page_and_wait(dev, p, NULL);
    if (ret) {
        LOG_ERR("Failed to read page %u", p);
        return -1;
    }

    ret = spi_nand_write_enable(dev->config.spi_dev);
    if (ret) {
        LOG_ERR("Failed to enable write, error: %d", ret);
        return -1;
    }

    ret = spi_nand_program_load(dev->config.spi_dev, data, 0, dev->page_size);
    if (ret) {
        LOG_ERR("Failed to load program, error: %d", ret);
        return -1;
    }

    ret = spi_nand_program_load(dev->config.spi_dev, (uint8_t *)&used_marker, dev->page_size + 2, 2);
    if (ret) {
        LOG_ERR("Failed to load used marker, error: %d", ret);
        return -1;
    }

    ret = program_execute_and_wait(dev, p, &status);
    if (ret) {
        LOG_ERR("Failed to execute program, error: %d", ret);
        return -1;
    }

    if ((status & STAT_PROGRAM_FAILED) != 0) {
        LOG_DBG("prog failed, page=%u", p);
        dhara_set_error(err, DHARA_E_BAD_BLOCK);
        return -1;
    }

    return 0;
}




/*
int dhara_nand_is_free(const struct dhara_nand *n, dhara_page_t p)
{
    spi_nand_flash_device_t *dev = __containerof(n, spi_nand_flash_device_t, dhara_nand);
    esp_err_t ret;
    uint16_t used_marker;

    ESP_GOTO_ON_ERROR(read_page_and_wait(dev, p, NULL), fail, TAG, "");
    ESP_GOTO_ON_ERROR(spi_nand_read(dev->config.device_handle, (uint8_t *)&used_marker,
                                    dev->page_size + 2, 2),
                      fail, TAG, "");

    ESP_LOGD(TAG, "is free, page=%"PRIu32", used_marker=%04x,", p, used_marker);
    return used_marker == 0xFFFF;
fail:
    ESP_LOGE(TAG, "Error in dhara_nand_is_free %d", ret);
    return 0;
}
*/
int dhara_nand_is_free(const struct dhara_nand *n, dhara_page_t p)
{
    spi_nand_flash_device_t *dev = CONTAINER_OF(n, spi_nand_flash_device_t, dhara_nand);
    int ret;
    uint16_t used_marker = 0;

    // Assume read_page_and_wait and spi_nand_read are adapted for Zephyr
    ret = read_page_and_wait(dev, p, NULL);
    if (ret) {
        LOG_ERR("Failed to read page %u", p);
        return 0; // Indicates error or not free
    }

    ret = spi_nand_read(dev->config.spi_dev, (uint8_t *)&used_marker, dev->page_size + 2, 2);
    if (ret) {
        LOG_ERR("Failed to read OOB area for page %u", p);
        return 0; // Indicates error or not free
    }

    LOG_DBG("Is free, page=%u, used_marker=%04x", p, used_marker);
    return used_marker == 0xFFFF; // Check against expected marker value for a free page
}





static int is_ecc_error(uint8_t status)
{
    return (status & STAT_ECC1) != 0 && (status & STAT_ECC0) == 0;
}
/*
int dhara_nand_read(const struct dhara_nand *n, dhara_page_t p, size_t offset, size_t length,
                    uint8_t *data, dhara_error_t *err)
{
    ESP_LOGV(TAG, "read, page=%"PRIu32", offset=%d, length=%d", p, offset, length);
    assert(p < n->num_blocks * (1 << n->log2_ppb));
    spi_nand_flash_device_t *dev = __containerof(n, spi_nand_flash_device_t, dhara_nand);
    esp_err_t ret;
    uint8_t status;

    ESP_GOTO_ON_ERROR(read_page_and_wait(dev, p, &status), fail, TAG, "");

    if (is_ecc_error(status)) {
        ESP_LOGD(TAG, "read ecc error, page=%"PRIu32"", p);
        dhara_set_error(err, DHARA_E_ECC);
        return -1;
    }

    ESP_GOTO_ON_ERROR(spi_nand_read(dev->config.device_handle, data, offset, length), fail, TAG, "");

    return 0;
fail:
    ESP_LOGE(TAG, "Error in dhara_nand_read %d", ret);
    return -1;
}
*/
int dhara_nand_read(const struct dhara_nand *n, dhara_page_t p, size_t offset, size_t length,
                    uint8_t *data, dhara_error_t *err)
{
    LOG_DBG("Read, page=%u, offset=%zu, length=%zu", p, offset, length);
    __ASSERT(p < n->num_blocks * (1 << n->log2_ppb), "Page out of range");
    spi_nand_flash_device_t *dev = CONTAINER_OF(n, spi_nand_flash_device_t, dhara_nand);
    int ret;
    uint8_t status;

    // Assuming read_page_and_wait is adapted for Zephyr and implemented elsewhere
    ret = read_page_and_wait(dev, p, &status);
    if (is_ecc_error(status)) {
        LOG_ERR("Failed to read page %u", p);
        dhara_set_error(err, DHARA_E_ECC); // Adjust error handling as necessary
        return -1;
    }

    ret = spi_nand_read(dev->config.spi_dev, data, offset, length);
    if (ret) {
        LOG_ERR("Failed to read data from page %u", p);
        //*err = DHARA_E_IO; // Adjust error handling as necessary
        return -1;
    }

    return 0;
}






/*
int dhara_nand_copy(const struct dhara_nand *n, dhara_page_t src, dhara_page_t dst, dhara_error_t *err)
{
    ESP_LOGD(TAG, "copy, src=%"PRIu32", dst=%"PRIu32"", src, dst);
    spi_nand_flash_device_t *dev = __containerof(n, spi_nand_flash_device_t, dhara_nand);
    esp_err_t ret;
    uint8_t status;

    ESP_GOTO_ON_ERROR(read_page_and_wait(dev, src, &status), fail, TAG, "");

    if (is_ecc_error(status)) {
        ESP_LOGD(TAG, "copy, ecc error");
        dhara_set_error(err, DHARA_E_ECC);
        return -1;
    }

    ESP_GOTO_ON_ERROR(spi_nand_write_enable(dev->config.device_handle), fail, TAG, "");
    ESP_GOTO_ON_ERROR(program_execute_and_wait(dev, dst, &status), fail, TAG, "");

    if ((status & STAT_PROGRAM_FAILED) != 0) {
        ESP_LOGD(TAG, "copy, prog failed");
        dhara_set_error(err, DHARA_E_BAD_BLOCK);
        return -1;
    }

    return 0;
fail:
    ESP_LOGE(TAG, "Error in dhara_nand_copy %d", ret);
    return -1;
}
*/

int dhara_nand_copy(const struct dhara_nand *n, dhara_page_t src, dhara_page_t dst, dhara_error_t *err)
{
    LOG_DBG("Copy, src=%u, dst=%u", src, dst);
    spi_nand_flash_device_t *dev = CONTAINER_OF(n, spi_nand_flash_device_t, dhara_nand);
    int ret;
    uint8_t status;

    // Assuming read_page_and_wait is adapted for Zephyr and implemented elsewhere
    ret = read_page_and_wait(dev, src, &status);
    if (ret) {
        LOG_ERR("Failed to read page %u", src);
        dhara_set_error(err, DHARA_E_ECC); // Adjust error handling as necessary
        return -1;
    }

    
    if (is_ecc_error(status)) {
        LOG_DBG("Copy, ECC error");
        dhara_set_error(err, DHARA_E_ECC);
        return -1;
    }

    // Assuming write_enable and program_execute_and_wait are adapted for Zephyr
    ret = spi_nand_write_enable(dev->config.spi_dev);
    if (ret) {
        LOG_ERR("Failed to enable write");
        return -1;
    }

    ret = program_execute_and_wait(dev, dst, &status);
    if (ret) {
        LOG_ERR("Program execute failed");
        return -1;
    }

    // Check for programming failure
    if ((status & STAT_PROGRAM_FAILED) != 0) {
        LOG_DBG("Copy, program failed");
        dhara_set_error(err, DHARA_E_BAD_BLOCK);
        return -1;
    }

    return 0;
}
