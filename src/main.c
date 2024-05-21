#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>

#include <zephyr/drivers/gpio.h>                                                                                                                                                     
#include <zephyr/drivers/spi.h>
#include <zephyr/fs/fs.h>
#include <zephyr/devicetree.h>
#include "zephyr/storage/disk_access.h"
#include <ff.h> 

#ifndef MAIN


#include "SPI_NAND_DHARA/spi_nand_oper.h"
#include "tests/spi_nand_oper_tests.h"
#include "tests/test_spi_nand_top_layer.h"
#include "tests/test_disk_access.h"
#include "vfs_NAND_flash.h"
#include "tests/vfs_test.h"
#include "tests/main_nand_tests.h"
#include "USB_mass_storage.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);



int  main(void)
{
	LOG_INF("My first breath as an IoT device");
	
	//  const struct spi_dt_spec spidev_dt = spi_nand_init();
	//  static struct disk_info nand_disk;
	//  int ret;

	//Test the SPI communication
	// ret = test_SPI_NAND_Communicator_all_tests(&spidev_dt);
	// if (ret != 0) {
    //     LOG_ERR("Communication tests failed, err: %d", ret);
    // }

	// ret = test_nand_top_layer(&spidev_dt);
	// if (ret != 0) {
    //     LOG_ERR("Top layer DHARA tests failed, err: %d", ret);
    // }

	//test_disk_initialize_status_read(&nand_disk);

	//test_vfs_NAND_flash();
	
	initialize_mass_storage_nand(); 
	test_all_main_nand_tests();
//	mount_nand_fs();

	return 0;

}

#endif // MAIN