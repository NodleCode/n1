/* Copyright (c) Nodle,Inc*/

#include <stdint.h>
#include <stdio.h>
#include <stdint.h>
#include "arch_system.h"
#include "user_periph_setup.h"
#include "gpio.h" 
#include "uart.h"
#include "uart_utils.h"
#include "spi.h"
#include "spi_flash.h"
#if defined (__DA14531__)
#include "rf_531.h"
#endif
#include "ibeacon.h"


uint8_t uuid[PROXIMITY_UUID_LEN] = {0};
uint16_t majorNo = 0;
uint16_t minorNo = 0;

#define DEBUG

/**
 ****************************************************************************************
 * @brief System start callback - gets called following a reset (but NOT on wakeup).
 * @return void
 ****************************************************************************************
*/
void ibeacon_on_init(void)
{
    /* If we have booted from flash then it will still be in active mode, power down to 
         reduce current consumption. */
    (void)spi_flash_power_down();	

    /* Set output power to maximum value (+2.5dBm) */
    rf_pa_pwr_set(RF_TX_PWR_LVL_PLUS_2d5);

    default_app_on_init();
}

/**
 ******************************************************************************************
 * @brief get_flash_data - This will initialize the flash and read the UUID, Major and Minor No
 * @return void
 ******************************************************************************************
*/
void get_flash_data (void)
{
	  uint16_t deviceId;
		uint8_t devID;
		uint8_t data[256]= {0};
		uint32_t actualSize = 0;
		uint32_t dataSize = 0;
		uint16_t i = 0;
		
		printf_string(UART, "\n\rFlash Data\r\n");
	 
	  // Release the SPI flash memory from power down
    spi_flash_release_from_power_down();

    // Disable the SPI flash memory protection (unprotect all sectors)
    spi_flash_configure_memory_protection(SPI_FLASH_MEM_PROT_NONE);
	
    // Try to auto-detect the SPI flash memory
    spi_flash_auto_detect(&devID);
	  spi_flash_read_rems_id(&deviceId);

    spi_cs_low();
    spi_cs_high();
		
	  spi_flash_read_data(data, FLASH_DATA_START_LOCATION, DATASIZE, &actualSize);
		memset (&uuid[0], 0, sizeof (uuid));
		memset (&majorNo, 0, sizeof (majorNo));
		memset (&minorNo, 0, sizeof (minorNo));
		
		memcpy (uuid, &data[0], PROXIMITY_UUID_LEN);
		memcpy (&majorNo, &data[PROXIMITY_UUID_LEN], MAJOR_MINOR_LEN);
		memcpy (&minorNo, &data[PROXIMITY_UUID_LEN + MAJOR_MINOR_LEN], MAJOR_MINOR_LEN);

#ifdef DEBUG
	
		for (i = 0; i < dataSize; i++)
    {
        printf_byte(UART, data[i]);
        printf_string(UART, " ");
    }
		
		for (i = 0; i < PROXIMITY_UUID_LEN; i++)
    {
        printf_byte(UART, uuid[i]);
        printf_string(UART, " ");
    }
		
    print_hword(UART, majorNo);
    printf_string(UART, " ");
		
    print_hword(UART, minorNo);
    printf_string(UART, " ");
#endif
		
}

/**
 ******************************************************************************************
 * @brief ibeacon_adv_start - This will start the iBeacon advertising
 * @return void
 ******************************************************************************************
*/
void ibeacon_adv_start(void)
{
    ibeacon_payload_type ibeacon;
    struct gapm_start_advertise_cmd *cmd = app_easy_gap_non_connectable_advertise_get_active();	
	
#ifdef DEBUG	
		printf_string(UART, "\n\riBeacon Advertising\r\n");
#endif
	
		/* Access SPI flash to get UUID, Major and Minor No */
		get_flash_data();
	
    /* Initialize the iBeacon advertising payload */
    ibeacon.flags[0]       = FLAG0;
    ibeacon.flags[1]       = FLAG1;
    ibeacon.flags[2]       = FLAG2;
    ibeacon.length         = IBEACON_LEN;
    ibeacon.type           = TYPE;
    ibeacon.company_id[0]  = COMPANY_ID0;
    ibeacon.company_id[1]  = COMPANY_ID1;
    ibeacon.beacon_type[0] = BEACON_TYPE0;
    ibeacon.beacon_type[1] = BEACON_TYPE1;

		memcpy (ibeacon.proximity_uuid, uuid, PROXIMITY_UUID_LEN);
    ibeacon.major = majorNo;
    ibeacon.minor = minorNo;
    ibeacon.measured_power = MEASURED_POWER;

    memcpy(cmd->info.host.adv_data, &ibeacon, sizeof(ibeacon_payload_type));
    cmd->info.host.adv_data_len = sizeof(ibeacon_payload_type);

    /* Set the advertising interval */
    cmd->intv_min = MS_TO_BLESLOTS(ADV_INTERVAL_ms);
    cmd->intv_max = MS_TO_BLESLOTS(ADV_INTERVAL_ms);

    /* Request advertising is started.. */
    app_easy_gap_non_connectable_advertise_start();
}

