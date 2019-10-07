#ifndef _BLUEVOICE_CONFIG_H_
#define _BLUEVOICE_CONFIG_H_

#include "compiler.h"


/* This file contains all the information needed to init the BlueNRG-1 stack. 
 * These constants and variables are used from the BlueNRG-1 stack to reserve RAM and FLASH 
 * according the application requests
 */


/* Default number of link */
#define MIN_NUM_LINK            1
/* Default number of GAP and GATT services */
#define DEFAULT_NUM_GATT_SERVICES   2
/* Default number of GAP and GATT attributes */
#define DEFAULT_NUM_GATT_ATTRIBUTES 9


/* Number of services requests from the BlueVoice demo */
#define NUM_APP_GATT_SERVICES 2

/* Number of attributes requests from the BlueVoice demo */
#define NUM_APP_GATT_ATTRIBUTES 9


/* Number of links needed for the demo: 1
 * Only 1 the default
 */
#define NUM_LINKS               (MIN_NUM_LINK)

/* Number of GATT attributes needed for the BlueVoice demo. */
#define NUM_GATT_ATTRIBUTES     (DEFAULT_NUM_GATT_ATTRIBUTES + NUM_APP_GATT_ATTRIBUTES)

/* Number of GATT services needed for the BlueVoice demo. */
#define NUM_GATT_SERVICES       (DEFAULT_NUM_GATT_SERVICES + NUM_APP_GATT_SERVICES)

/* Array size for the attribte value */
#define ATT_VALUE_ARRAY_SIZE    (1344)

/* Flash security database size */
#define FLASH_SEC_DB_SIZE       (0x400)

/* Flash server database size */
#define FLASH_SERVER_DB_SIZE    (0x400)

/* RAM reserved to manage all the data stack according the number of links,
 * number of services, number of attributes and attribute value length
 */
NO_INIT(uint32_t dyn_alloc_a[TOTAL_BUFFER_SIZE(NUM_LINKS,NUM_GATT_ATTRIBUTES,NUM_GATT_SERVICES,ATT_VALUE_ARRAY_SIZE)>>2]);

/* FLASH reserved to store all the security database information and
 * and the server database information
 */
ALIGN(4)
SECTION(".noinit.stacklib_flash_data")
NOLOAD(const uint32_t stacklib_flash_data[TOTAL_FLASH_BUFFER_SIZE(FLASH_SEC_DB_SIZE, FLASH_SERVER_DB_SIZE)>>2]);
/* FLASH reserved to store: security root keys, static random address, public address */
ALIGN(4)
SECTION(".noinit.stacklib_stored_device_id_data")
NOLOAD(const uint8_t stacklib_stored_device_id_data[56]);

/* Maximum duration of the connection event */
#define MAX_CONN_EVENT_LENGTH 0xFFFFFFFF

/* Sleep clock accuracy in Slave mode 100 ppm */
#define SLAVE_SLEEP_CLOCK_ACCURACY 100

/* Sleep clock accuracy in Master mode 100 ppm */
#define MASTER_SLEEP_CLOCK_ACCURACY 3

/* Low Speed Oscillator source */
#if (LS_SOURCE == LS_SOURCE_INTERNAL_RO)
#define LOW_SPEED_SOURCE  1 // Internal RO
#else
#define LOW_SPEED_SOURCE  0 // External 32 KHz
#endif

/* High Speed start up time */
#define HS_STARTUP_TIME 0x0107 // 642 us

/* Low level hardware configuration data for the device */
#define CONFIG_TABLE            \
{                               \
  NULL,                         \
  MAX_CONN_EVENT_LENGTH,        \
  SLAVE_SLEEP_CLOCK_ACCURACY,   \
  MASTER_SLEEP_CLOCK_ACCURACY,  \
  LOW_SPEED_SOURCE,             \
  HS_STARTUP_TIME               \
}

/* This structure contains memory and low level hardware configuration data for the device */
const BlueNRG_Stack_Initialization_t BlueNRG_Stack_Init_params = {
    (uint8_t*)stacklib_flash_data,
    FLASH_SEC_DB_SIZE,
    FLASH_SERVER_DB_SIZE,
    (uint8_t*)stacklib_stored_device_id_data,
    (uint8_t*)dyn_alloc_a,
    NUM_GATT_ATTRIBUTES,
    NUM_GATT_SERVICES,
    ATT_VALUE_ARRAY_SIZE,
    NUM_LINKS,
    CONFIG_TABLE
};

#endif // _BLUEVOICEDEMO_CONFIG_H_
