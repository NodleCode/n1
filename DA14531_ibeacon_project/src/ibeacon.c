/* Copyright (c) Nodle,Inc*/

#include <stdint.h>
#include <stdio.h>
#include <stdint.h>
#include "arch_system.h"
#include "gpio.h" 
#include "uart.h"
#include "uart_utils.h"
#include "spi.h"
#include "spi_flash.h"
#include "syscntl.h"
#if defined (__DA14531__)
#include "rf_531.h"
#endif
#include "ibeacon.h"

#define CFG_PRINTF_UART2 /* Enable UTX on the Dev Kit */
#define DEBUG /* This writes the debug statements to UART2 */

uint8_t uuid[PROXIMITY_UUID_LEN] = {0};
uint16_t majorNo = 0;
uint16_t minorNo = 0;


#if defined (CFG_PRINTF_UART2)
// Configuration struct for UART2
static const uart_cfg_t uart_cfg = {
    .baud_rate = UART_BAUDRATE_115200,
    .data_bits = UART_DATABITS_8,
    .parity = UART_PARITY_NONE,
    .stop_bits = UART_STOPBITS_1,
    .auto_flow_control = UART_AFCE_DIS,
    .use_fifo = UART_FIFO_EN,
    .tx_fifo_tr_lvl = UART_TX_FIFO_LEVEL_0,
    .rx_fifo_tr_lvl = UART_RX_FIFO_LEVEL_0,
    .intr_priority = 2,
};
#endif

/* Default SPI configuration */
static const spi_cfg_t spi_cfg = {
    .spi_ms = SPI_MS_MODE_MASTER,
    .spi_cp = SPI_CP_MODE_0,
    .spi_speed = SPI_SPEED_MODE_4MHz,
    .spi_wsz = SPI_MODE_8BIT,
    .spi_cs = SPI_CS_0,
    .cs_pad.port = GPIO_PORT_0,
    .cs_pad.pin = GPIO_PIN_1,
#if defined (__DA14531__)
    .spi_capture = SPI_MASTER_EDGE_CAPTURE,
#endif
};


static const spi_flash_cfg_t spi_flash_cfg = {
    .dev_index = MX25R2035F_DEV_INDEX,
    .jedec_id  = MX25V2035F_JEDEC_ID,
    .chip_size = MX25V2035F_CHIP_SIZE,
};

/**
 ****************************************************************************************
 * @brief GPIO Configuration
 * @return void
 ****************************************************************************************
*/
void configure_GPIOs(void)
{

#if defined (__DA14531__)
    // Configure pins used for SPI flash interface
    GPIO_Disable_HW_Reset();	
#endif
    
    GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_1, OUTPUT, PID_SPI_EN, true);
    GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_4, OUTPUT, PID_SPI_CLK, false);
    GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_0, OUTPUT, PID_SPI_DO, false);
    GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_3, INPUT, PID_SPI_DI, false);
	
#if defined (CFG_PRINTF_UART2)
    // Configure UART2 TX Pad
    GPIO_ConfigurePin(GPIO_PORT_0, GPIO_PIN_6, OUTPUT, PID_UART2_TX, false);
#endif

}


#if DEVELOPMENT_DEBUG
/**
 ****************************************************************************************
 * @brief GPIO reserve
 * @return void
 ****************************************************************************************
*/
void GPIO_reservations(void)
{

	  RESERVE_GPIO(SPI_FLASH_CS, GPIO_PORT_0, GPIO_PIN_1, PID_SPI_EN);
	  RESERVE_GPIO(SPI_FLASH_CLK, GPIO_PORT_0, GPIO_PIN_4, PID_SPI_CLK);
	  RESERVE_GPIO(SPI_FLASH_DO, GPIO_PORT_0, GPIO_PIN_0, PID_SPI_DO);
	  RESERVE_GPIO(SPI_FLASH_DI, GPIO_PORT_0, GPIO_PIN_3, PID_SPI_DI);
    
#if defined (CFG_PRINTF_UART2)
    RESERVE_GPIO(UART2_TX, GPIO_PORT_0, GPIO_PIN_6, PID_UART2_TX);
#endif
}

#endif

/**
 ****************************************************************************************
 * @brief Initialize peripherals
 * @return void
 ****************************************************************************************
*/
void periph_init(void)
{
    // In Boost mode enable the DCDC converter to supply VBAT_HIGH for the used GPIOs
    syscntl_dcdc_turn_on_in_boost(SYSCNTL_DCDC_LEVEL_3V0);

    // Initialize peripherals
#if defined (CFG_PRINTF_UART2)
    // Initialize UART2
    uart_initialize(UART2, &uart_cfg);
#endif

  	// Initialize interface to SPI flash so we can use it from within the application
    spi_flash_configure_env(&spi_flash_cfg);
    spi_initialize(&spi_cfg);

    // GPIO Configuration
    configure_GPIOs();

    // Enable the pads
    GPIO_set_pad_latch_en(true);
}


/**
 ****************************************************************************************
 * @brief System start callback - gets called following a reset
 * @return void
 ****************************************************************************************
*/
void ibeacon_init(void)
{
    /* Reduce power consumption */
    spi_flash_power_down();	

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
		
#ifdef DEBUG		
		printf_string(UART2, "\n\rFlash Data\r\n");
#endif	 
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
        printf_byte(UART2, data[i]);
        printf_string(UART2, " ");
    }
		
		for (i = 0; i < PROXIMITY_UUID_LEN; i++)
    {
        printf_byte(UART2, uuid[i]);
        printf_string(UART2, " ");
    }
		
    print_hword(UART2, majorNo);
    printf_string(UART2, " ");
		
    print_hword(UART2, minorNo);
    printf_string(UART2, " ");
#endif
		
}

/**
 ******************************************************************************************
 * @brief ibeacon_adv_start - This will start the iBeacon advertising
 * @return void
 ******************************************************************************************
*/
void ibeacon_start(void)
{
    ibeacon_payload_type ibeacon;
    struct gapm_start_advertise_cmd *adv = app_easy_gap_non_connectable_advertise_get_active();	
	
#ifdef DEBUG	
		printf_string(UART2, "\n\riBeacon Advertising\r\n");
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

    memcpy(adv->info.host.adv_data, &ibeacon, sizeof(ibeacon_payload_type));
    adv->info.host.adv_data_len = sizeof(ibeacon_payload_type);

    /* Set the advertising interval */
    adv->intv_min = MS_TO_BLESLOTS(ADVERT_INT_MS);
    adv->intv_max = MS_TO_BLESLOTS(ADVERT_INT_MS);

    /* Request advertising is started.. */
    app_easy_gap_non_connectable_advertise_start();
}

