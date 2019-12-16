#include <stddef.h>
#include <string.h>
#include "peripheral_mngr_app.h"

// this is the unique ID we assign to the device
#define BLE_DEVICE_ID '0','0','8'
#define BLE_LOCAL_NAME 'O','W','L','-','B','i','o','S','i','g','n','a','l','s','-','P',BLE_DEVICE_ID

#define MIDI_SERVICE_UUID 			0x00,0xC7,0xC4,0x4E,0xE3,0x6C,0x51,0xA7,0x33,0x4B,0xE8,0xED,0x5A,0x0E,0xB8,0x03
#define APP_READY               (0x00)
#define APP_BLUEVOICE_ENABLE    (0x01)
#define APP_INERTIAL_ENABLE     (0x02)

extern volatile uint16_t MidiHandle;
volatile uint16_t MidiServiceHandle;
volatile uint16_t conn_handle;
volatile uint8_t  APP_PER_enabled = APP_READY;

/**
 * Each Bluetooth device has a unique 48-bit Bluetooth device address (BD_ADDR). 
 * The address shall be a 48-bit extended unique identifier (EUI-48) created in 
 * accordance with "Universal address" of the IEEE 802-2014 standard.
 * NAP: 2 MSB bytes.
 * Abbreviated as Non-significant Address Part.
 * Assigned by the IEEE (Institute of Electrical and Electronics Engineers).
 * UAP: 1 byte.
 * Abbreviated as Upper Address Part.
 * Assigned by the IEEE (Institute of Electrical and Electronics Engineers).
 * LAP: 3 LSB bytes.
 * Abbreviated as Lower Address Part.
 * It's transmitted with every packet as part of packet header.
 * The LAP and the UAP make the significant address part (SAP) of the Bluetooth Address.
 * The upper half of a Bluetooth Address (most-significant 24 bits) is so called 
 * Organizationally Unique Identifier (OUI). It can be used to determine the manufacturer of a device
 * (Bluetooth MAC Address Lookup form). OUI prefixes are assigned by the IEEE.
 * The BD_ADDR may take any values except those that would have any of the 64 reserved LAP values for general and dedicated inquiries.
 * 16-bit company identifier 0x0030 ST Microelectronics
 */

/* uint8_t PERIPHERAL_BDADDR[] = {0x55, 0x11, 0x07, 0x01, 0x16, 0xE2};  */
uint8_t PERIPHERAL_BDADDR[] = {BLE_DEVICE_ID, 0x01, 0x16, 0xE2}; // reverse MAC address order


static const uint8_t midi_service_uuid[16] 	= { MIDI_SERVICE_UUID };
/* 0x00,0xC7,0xC4,0x4E,0xE3,0x6C, 0x51,0xA7, 0x33,0x4B, 0xE8,0xED, 0x5A,0x0E,0xB8,0x03}; */

volatile uint8_t APP_PER_state = APP_STATUS_ADVERTISEMENT;

static uint16_t service_handle, dev_name_char_handle, appearance_char_handle;
/**
 * @brief  BlueNRG-1 Initialization.
 * @param  None.
 * @retval APP_Status: APP_SUCCESS if the configuration is ok, APP_ERROR otherwise.
 */
APP_Status PER_APP_Init_BLE(void)
{  
  aci_hal_write_config_data(CONFIG_DATA_PUBADDR_OFFSET, CONFIG_DATA_PUBADDR_LEN, PERIPHERAL_BDADDR);
  
  // Set radio power level
  aci_hal_set_tx_power_level(1, 7); // max power?
  // When the system starts up or reboots, the default TX power level will be used, which is the maximum value of 8dBm
  
  // GATT init
  aci_gatt_init();
 
  // GAP init
  aci_gap_init(GAP_PERIPHERAL_ROLE, 0, 0x07, &service_handle, &dev_name_char_handle, &appearance_char_handle);

  // Set authentication requirements
  aci_gap_set_authentication_requirement(MITM_PROTECTION_REQUIRED, OOB_AUTH_DATA_ABSENT, NULL, 7, 16, USE_FIXED_PIN_FOR_PAIRING, 123456, BONDING);
                               
  return APP_SUCCESS;
}

/**
 * @brief  BLE service and characteristics initialization.
 * @param  None.
 * @retval APP_Status: APP_SUCCESS if the configuration is ok, APP_ERROR otherwise.
 */
APP_Status PER_APP_Service_Init(void)
{

	// Add  Midi service and handle
  aci_gatt_add_service(UUID_TYPE_128,
		       (Service_UUID_t *) midi_service_uuid, PRIMARY_SERVICE, 10,
		       (uint16_t*)&MidiServiceHandle);
  MIDI_APP_add_char(MidiServiceHandle);	
  return APP_SUCCESS;
}

/**
 * @brief  This function is called from the server in order set advertisement mode.
 * @param  None
 * @retval APP_Status: APP_SUCCESS if the configuration is ok, APP_ERROR otherwise.
 */
APP_Status PER_APP_Advertise(void)
{
  uint8_t ret = 0;
  uint8_t local_name[] =
  {
   /* AD_TYPE_SHORTENED_LOCAL_NAME, 'O','W','L','-','B','I','O', */
   AD_TYPE_COMPLETE_LOCAL_NAME, BLE_LOCAL_NAME
  };
	
  // Add scan response data
  uint8_t midi_scan_rsp[] = {
    0x11, AD_TYPE_128_BIT_SERV_UUID_CMPLT_LIST, MIDI_SERVICE_UUID
  };
  ret = hci_le_set_scan_response_data(sizeof(midi_scan_rsp), midi_scan_rsp);
		
  // Configure GAP
  ret |= aci_gap_set_discoverable(ADV_IND, 0x20,0x50, PUBLIC_ADDR, NO_WHITE_LIST_USE, sizeof(local_name), local_name, 0, NULL, 0, 0);

  // Send advertising data
/* #define NAME_WEAR 'O', 'W', 'L', '-', 'B', 'I', 'O' */
/*   uint8_t manuf_data[20] = { */
/*   2,0x0A,0x00, /\* AD_TYPE_TX_POWER_LEVEL *\/ */
/*   8,0x09,NAME_WEAR, /\* Complete Name *\/ */
/*   7,0xFF,0x01,      /\* SKD version *\/ */
/*          0x00, */
/*          0x48,      /\* AudioSync+AudioData *\/ */
/*          0xC0,      /\* Acc+Gyr *\/ */
/*          0x00, */
/*          0x00 */
/*   }; */
  /* uint8_t manuf_data[] = { */
  /*   8, AD_TYPE_SHORTENED_LOCAL_NAME, 'O', 'W', 'L', '-', 'B', 'I', 'O' */
  /* }; // not sure why this returns a 0x41 BLE_STATUS_FAILED */
  /* uint8_t manuf_data[] = { */
  /*   20, AD_TYPE_COMPLETE_LOCAL_NAME, 'O','W','L','-','B','i','o','S','i','g','n','a','l','s', */
  /*   '-','P','0','0','0' */
  /* }; // not sure why this returns a 0x41 BLE_STATUS_FAILED */
  /* uint8_t manuf_data[] = { */
  /*    /\* AD_TYPE_SHORTENED_LOCAL_NAME, 'O','W','L','-','B','I','O', *\/ */
  /*    AD_TYPE_COMPLETE_LOCAL_NAME, BLE_LOCAL_NAME */
  /* }; */
  /* aci_gap_update_adv_data(sizeof(manuf_data), manuf_data); */

  if (ret != BLE_STATUS_SUCCESS)
    return APP_ERROR;  
  return APP_SUCCESS;
}

/*******************************************************************************
 * Function Name  : hci_le_connection_complete_event.
 * Description    : This event indicates that a new connection has been created.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void hci_le_connection_complete_event(uint8_t  Status,
                                      uint16_t Connection_Handle,
                                      uint8_t  Role,
                                      uint8_t  Peer_Address_Type,
                                      uint8_t  Peer_Address[6],
                                      uint16_t Conn_Interval,
                                      uint16_t Conn_Latency,
                                      uint16_t Supervision_Timeout,
                                      uint8_t  Master_Clock_Accuracy)

{ 
  //BSP_LED_On(LED1);

  conn_handle = Connection_Handle;
  int ret = aci_l2cap_connection_parameter_update_req(conn_handle,
                                                9 	/* interval_min*/,
                                                9 	/* interval_max */,
                                                0   /* slave_latency */,
                                                400 /*timeout_multiplier*/);  
  if (ret != BLE_STATUS_SUCCESS)
  {
    while (1); // todo: replace with reset or reinit
  }
  
  APP_PER_state = APP_STATUS_CONNECTED;
    
}/* end hci_le_connection_complete_event() */


/*******************************************************************************
 * Function Name  : hci_disconnection_complete_event_isr.
 * Description    : This event occurs when a connection is terminated.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void hci_disconnection_complete_event(uint8_t  Status,
                                      uint16_t Connection_Handle,
                                      uint8_t  Reason)
{
 
  /* Make the device connectable again. */
  APP_PER_state = APP_STATUS_ADVERTISEMENT;
  
  /*Set module in advertise mode*/
  APP_Status status = PER_APP_Advertise();
  /* aci_gap_update_adv_data() in PER_APP_Advertise() always returns BLE_STATUS_INVALID_PARAMS */
  /* if(status != APP_SUCCESS) */
  /* { */
  /*   PER_APP_Error_Handler(); */
  /* } */

}/* end hci_disconnection_complete_event_isr() */


/*******************************************************************************
 * Function Name  : aci_gatt_attribute_modified_event.
 * Description    : This event occurs when an attribute is modified.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
uint16_t attr_handle = 0;
uint8_t midi_handle_value_len = 0;
uint8_t midi_handle_value[4] = {};
void aci_gatt_attribute_modified_event(uint16_t Connection_Handle,
                                       uint16_t Attr_Handle,
                                       uint16_t Offset,
                                       uint8_t  Attr_Data_Length,
                                       uint8_t  Attr_Data[])
{
  /* if((Attr_Handle == tx_handle.CharAudioHandle+2) || (Attr_Handle == tx_handle.CharAudioSyncHandle+2)) */
  /* { */
  /*   if(Attr_Data[0] == 0x01) */
  /*   {    */
  /*     if(!audio_streaming_active) */
  /*     { */
  /*       BV_APP_StartStop_ctrl(); */
  /*     } */
      
  /*     APP_PER_enabled = APP_BLUEVOICE_ENABLE; */
  /*     /\* BluevoiceADPCM_BNRG1_AttributeModified_CB(Attr_Handle, Attr_Data_Length, Attr_Data); *\/ */
  /*   } */
  /*   else if(Attr_Data[0] == 0x00) */
  /*   { */
  /*     if(APP_PER_enabled == APP_BLUEVOICE_ENABLE) */
  /*     { */
  /*       APP_PER_enabled = APP_READY; */
  /*       if(audio_streaming_active) */
  /*       { */
  /*         BV_APP_StartStop_ctrl(); */
  /*       } */
  /*     } */
  /*   } */
  /* }      */
  /* if(Attr_Handle == ClassificationHandle+2) */
  /* { */
  /* }  */
  /* if(Attr_Handle == MidiServiceHandle) */
  /* { */
  /* } */

  // todo: after MODE_APP_DataRead, need to catch data here
  if(Attr_Handle == MidiServiceHandle+2){
    midi_handle_value_len = Attr_Data_Length;
    if(Attr_Data_Length <= sizeof(midi_handle_value))
      memcpy(midi_handle_value, Attr_Data, Attr_Data_Length);
    else
      memcpy(midi_handle_value, Attr_Data, sizeof(midi_handle_value));
  }
  attr_handle = Attr_Handle;
}

void aci_gatt_tx_pool_available_event(uint16_t Connection_Handle,
                                      uint16_t Available_Buffers)
{       
  /* It allows to notify when at least 2 GATT TX buffers are available */
  /* tx_buffer_full = 0; */
} 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
