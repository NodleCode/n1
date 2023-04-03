/* Copyright (c) Nodle,Inc*/

#ifndef _IBEACON_H_
#define _IBEACON_H_

#include <stdint.h>
#include "app_api.h"
#include "app_bass.h"
#include "app_findme.h"
#include "app_proxr.h"
#include "app_suotar.h"
#include "app_callback.h"
#include "app_prf_types.h"
#include "app_default_handlers.h"

#define FLAG0 	0x02
#define FLAG1 	0x01
#define FLAG2 	0x06
#define LEN 		0x1A
#define TYPE 		0xFF
#define COMPANY_ID0	0x4C
#define COMPANY_ID1	0x00
#define BEACON_TYPE0 0x02
#define BEACON_TYPE1	0x15

#define FLAGS_LEN 3
#define COMPANY_ID_LEN	2
#define BEACON_TYPE_LEN	2
#define PROXIMITY_UUID_LEN 16
#define DATASIZE 24
#define MAJOR_MINOR_LEN 2
#define IBEACON_LEN 0x1A
#define MEASURED_POWER 0xC6
#define ADVERT_INT_MS 100 
#define FLASH_DATA_START_LOCATION 0x10000

/* Apple Proximity Beacon Advertising Packet Type */
typedef struct __attribute__ ((__packed__)) {
	uint8_t flags[FLAGS_LEN];
	uint8_t length;
	uint8_t type;
	uint8_t company_id[COMPANY_ID_LEN];
	uint8_t beacon_type[BEACON_TYPE_LEN];
	uint8_t proximity_uuid[PROXIMITY_UUID_LEN];
	uint16_t major;
	uint16_t minor;
	uint8_t measured_power;
} ibeacon_payload_type;

void ibeacon_start(void);
void ibeacon_init(void);
void default_app_on_set_dev_config_complete(void);
void default_app_on_db_init_complete( void );
void default_app_generate_unique_static_random_addr(struct bd_addr *addr);

#endif
