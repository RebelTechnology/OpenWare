#include "inertial_app.h"
#include "BlueNRG1_conf.h"
#include "ble_const.h" 
 
volatile uint16_t ClassificationHandle = 0, ModeHandle = 0, MidiHandle = 0;
static const uint8_t Classification_char_uuid[16] = {0x1b,0xc5,0xd5,0xa5,0x02,0x00,0x36,0xac,0xe1,0x11,0x01,0x00,0x00,0x00,0x00,0x01};
static const uint8_t Mode_char_uuid[16] 					= {0x1b,0xc5,0xd5,0xa5,0x02,0x00,0x36,0xac,0xe1,0x11,0x01,0x00,0x00,0x00,0x00,0x02};
static const uint8_t MIDI_IO_char_uuid[16] 				= {0xF3,0x6B,0x10,0x9D, 0x66,0xF2, 0xA9,0xA1, 0x12,0x41, 0x68,0x38,0xDB,0xE5,0x72,0x77};

uint16_t usiTimerVal;
/**
 * @brief  This function is called to add Classification characteristics.
 * @param  service_handle: handle of the service
 * @retval INERTIAL_APP_Status: INERTIAL_APP_SUCCESS if the configuration is ok, INERTIAL_APP_ERROR otherwise.
 */

// _____ Classification Handle _________________________________________________________________________________________________________
SCUDO_APP_Status CLASSIFICATION_APP_add_char(uint16_t service_handle)
{      
  uint8_t ret = aci_gatt_add_char(service_handle,
                            UUID_TYPE_128, (Char_UUID_t *) Classification_char_uuid,
                            1, CHAR_PROP_READ/*|CHAR_PROP_NOTIFY*/, ATTR_PERMISSION_NONE, GATT_DONT_NOTIFY_EVENTS, 16, 1,
                            (uint16_t*)&ClassificationHandle);
                        
  if (ret != BLE_STATUS_SUCCESS)
  {
    return SCUDO_APP_ERROR;
  }

  return SCUDO_APP_SUCCESS;
}

SCUDO_APP_Status CLASSIFICATION_APP_DataUpdate(uint16_t service_handle, uint8_t value)
{  
	uint8_t buff[1];  
	
	buff[0] = value;
	
	if(aci_gatt_update_char_value(service_handle, ClassificationHandle, 0, sizeof buff, buff)==BLE_STATUS_INSUFFICIENT_RESOURCES)
	{
		return SCUDO_APP_ERROR;
	}
	return SCUDO_APP_SUCCESS;	
}

// _____ Mode Handle ______________________________________________________________________________________________________________
SCUDO_APP_Status MODE_APP_add_char(uint16_t service_handle)
{      
  uint8_t ret = aci_gatt_add_char(service_handle,
                            UUID_TYPE_128, (Char_UUID_t *) Mode_char_uuid,
                            1, CHAR_PROP_WRITE|CHAR_PROP_READ, ATTR_PERMISSION_NONE, GATT_NOTIFY_ATTRIBUTE_WRITE, 16, 0,
                            (uint16_t*)&ModeHandle);
                        
  if (ret != BLE_STATUS_SUCCESS)
  {
    return SCUDO_APP_ERROR;
  }

  return SCUDO_APP_SUCCESS;
}

uint8_t MODE_APP_DataRead(uint16_t service_handle)
{  
	if(aci_gatt_read_char_value(service_handle, ModeHandle)==BLE_STATUS_INSUFFICIENT_RESOURCES)
	{
		return SCUDO_APP_ERROR;
	}
	return SCUDO_APP_SUCCESS;	
}

// _____ MIDI Handle ______________________________________________________________________________________________________________
SCUDO_APP_Status MIDI_APP_add_char(uint16_t service_handle)
{      
  uint8_t ret = aci_gatt_add_char(service_handle,
                            UUID_TYPE_128, (Char_UUID_t *) MIDI_IO_char_uuid,
                            5, CHAR_PROP_READ|CHAR_PROP_NOTIFY|CHAR_PROP_WRITE_WITHOUT_RESP, ATTR_PERMISSION_ENCRY_WRITE, GATT_NOTIFY_ATTRIBUTE_WRITE, 16, 1,
                            (uint16_t*)&MidiHandle);
                        
  if (ret != BLE_STATUS_SUCCESS)
  {
    return SCUDO_APP_ERROR;
  }

  return SCUDO_APP_SUCCESS;
}


SCUDO_APP_Status MIDI_APP_DataUpdate(uint16_t service_handle, uint8_t channel, uint8_t note, uint8_t vector)
{  
	uint8_t rgBuff[5]; 
	
	// Ensure channel is not 0
	if (!channel) channel = 1;
	
	// Build Tx packet
	rgBuff[0] = 0x80 | ((usiTimerVal >> 7) & 0x3F);
	rgBuff[1] = 0x80 | (usiTimerVal & 0x007F);
	rgBuff[2] = 0x90 | ((channel-1) & 0x0F);
	rgBuff[3] = note   & 0x7F;
	rgBuff[4] = vector & 0x7F;
	
	// Update value
	if(aci_gatt_update_char_value(service_handle, MidiHandle, 0, sizeof rgBuff, rgBuff)==BLE_STATUS_INSUFFICIENT_RESOURCES)
	{
		return SCUDO_APP_ERROR;
	}
	return SCUDO_APP_SUCCESS;	
}

SCUDO_APP_Status MIDI_APP_Passthrough(uint16_t service_handle, uint8_t* data)
{  
	uint8_t rgBuff[5]; 
	
	// Build Tx packet
	rgBuff[0] = 0x80 | ((usiTimerVal >> 7) & 0x3F);
	rgBuff[1] = 0x80 | (usiTimerVal & 0x007F);
	rgBuff[2] = data[1];
	rgBuff[3] = data[2];
	rgBuff[4] = data[3];
	
	// Update value
	tBleStatus ret;
	ret = aci_gatt_update_char_value(service_handle, MidiHandle, 0, sizeof rgBuff, rgBuff);
	if(ret == BLE_STATUS_INSUFFICIENT_RESOURCES)
	  return SCUDO_APP_ERROR;
	return SCUDO_APP_SUCCESS;	
}





/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
