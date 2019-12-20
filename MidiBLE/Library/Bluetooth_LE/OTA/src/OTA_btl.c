/**
  ******************************************************************************
  * @file    OTA_btl.c
  * @author  AMS - VMA RF Application team
  * @version V1.1.0
  * @date    29-Juanuary-2016
  * @brief   BLE Over The Air FW (OTA) upgrade implementation
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "BlueNRG1_conf.h"
#include "bluenrg1_stack.h"
#include "ble_const.h"
#include "SDK_EVAL_Config.h"
#include "osal.h"
#include "OTA_btl.h"

#ifdef DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define OTA_LED LED3 /* LED turned ON  OTA session is ongoing */

/** Define variables for user choice:
 *  For essential amount of status information during OTA bootloader session 
 *  let ST_OTA_BTL_MINIMAL_ECHO be defined (strongly suggested)
 */
#define ST_OTA_BTL_MINIMAL_ECHO

/* The following defines **MUST NOT** be modified for proper operation of the current OTA BTL release */
#define PAGE_SIZE 2048 // Flash page size
#define BUF_SIZE 128 //It's equal to the data lenght of received packet (buffer to hold the received notification/s to be written on flash)
#define BYTE_INCREMENT 16 // It's equal to max flash size we can write: 16 with Flash Burst Write
#define NOTIFICATION_WINDOW 8
#define NOTIFICATION_INTERVAL(x) (((x) == 1) || ((x) == 3)) ? 1 : (NOTIFICATION_WINDOW) //3: backward compatibility with old OTA client 

/* OTA bootloades notification error codes */
#define OTA_SUCCESS            0x0000
#define OTA_FLASH_VERIFY_ERROR 0x003C
#define OTA_FLASH_WRITE_ERROR  0x00FF
#define OTA_SEQUENCE_ERROR     0x00F0
#define OTA_CHECKSUM_ERROR     0x000F

#define OTA_WRITE_GUARD_TIME 2 //2ms 
   
/* Characteristic handles */
uint16_t btlServHandle, btlImageCharHandle, btlNewImageCharHandle, btlNewImageTUContentCharHandle, btlExpectedImageTUSeqNumberCharHandle;

/* OTA service & characteristic UUID 128:

OTA service: 8a97f7c0-8506-11e3-baa7-0800200c9a66
Image Characteristic: 122e8cc0-8508-11e3-baa7-0800200c9a66
New Image Characteristic: 210f99f0-8508-11e3-baa7-0800200c9a66
New Image TU Content Characteristic:2691aa80-8508-11e3-baa7-0800200c9a66
ExpectedImageTUSeqNumber Characteristic: 2bdc5760-8508-11e3-baa7-0800200c9a66
*/

#define COPY_UUID_128(uuid_struct, uuid_15, uuid_14, uuid_13, uuid_12, uuid_11, uuid_10, uuid_9, uuid_8, uuid_7, uuid_6, uuid_5, uuid_4, uuid_3, uuid_2, uuid_1, uuid_0) \
do {\
    uuid_struct[0] = uuid_0; uuid_struct[1] = uuid_1; uuid_struct[2] = uuid_2; uuid_struct[3] = uuid_3; \
        uuid_struct[4] = uuid_4; uuid_struct[5] = uuid_5; uuid_struct[6] = uuid_6; uuid_struct[7] = uuid_7; \
            uuid_struct[8] = uuid_8; uuid_struct[9] = uuid_9; uuid_struct[10] = uuid_10; uuid_struct[11] = uuid_11; \
                uuid_struct[12] = uuid_12; uuid_struct[13] = uuid_13; uuid_struct[14] = uuid_14; uuid_struct[15] = uuid_15; \
}while(0)

#define COPY_BTL_SERVICE_UUID(uuid_struct)                   COPY_UUID_128(uuid_struct,0x8a,0x97,0xf7,0xc0,0x85,0x06,0x11,0xe3,0xba,0xa7,0x08,0x00,0x20,0x0c,0x9a,0x66)
#define COPY_IMAGE_CHAR_UUID(uuid_struct)                    COPY_UUID_128(uuid_struct,0x12,0x2e,0x8c,0xc0,0x85,0x08,0x11,0xe3,0xba,0xa7,0x08,0x00,0x20,0x0c,0x9a,0x66)
#define COPY_NEW_IMAGE_CHAR_UUID(uuid_struct)                COPY_UUID_128(uuid_struct,0x21,0x0f,0x99,0xf0,0x85,0x08,0x11,0xe3,0xba,0xa7,0x08,0x00,0x20,0x0c,0x9a,0x66)
#define COPY_NEW_IMAGE_TU_CONTENT_CHAR_UUID(uuid_struct)     COPY_UUID_128(uuid_struct,0x26,0x91,0xaa,0x80,0x85,0x08,0x11,0xe3,0xba,0xa7,0x08,0x00,0x20,0x0c,0x9a,0x66)
#define COPY_EXPECTED_IMAGE_TU_SEQNUM_CHAR_UUID(uuid_struct) COPY_UUID_128(uuid_struct,0x2b,0xdc,0x57,0x60,0x85,0x08,0x11,0xe3,0xba,0xa7,0x08,0x00,0x20,0x0c,0x9a,0x66)

struct
{
    uint16_t  replyCounter;
    uint16_t  errCode;    
} notification;

uint8_t imageBuffer[BUF_SIZE];

uint8_t BTLServiceUUID4Scan[18]= {0x11,0x06,0x8a,0x97,0xf7,0xc0,0x85,0x06,0x11,0xe3,0xba,0xa7,0x08,0x00,0x20,0x0c,0x9a,0x66}; 

/* Let the application know whether we are in the middle of a bootloading session through a global status variable */
//uint8_t bootloadingOngoing = 0;

static uint8_t bootloadingCompleted = 0;
static uint8_t bootloadingCompleted_end = 0; 
static uint8_t ota_allow_jump = 0; 
  
/* UUIDS */
static Service_UUID_t ota_service_uuid;
static Char_UUID_t ota_char_uuid;
static uint16_t numPages = 0; 
static uint32_t imageBase = 0;  
static volatile uint8_t do_erase_flash=0;  
static volatile uint8_t erase_flash_done=0;

static volatile uint8_t ota_service_is_disconnected=0;
// static uint16_t PageNumber = 0; 
    
static volatile uint8_t detected_error=0;
static volatile uint16_t write_counter=0; 

static volatile uint8_t ota_write_data = 0; 
static volatile uint8_t ota_do_notification = 0; 

static uint32_t imageSize = 0; 
static uint16_t bufPointer = 0;
static uint32_t totalBytesWritten = 0;
static uint8_t checksum = 0;
static uint32_t currentWriteAddress = 0;
static uint16_t expectedSeqNum = 0;

static uint8_t notification_range = NOTIFICATION_WINDOW; 
static uint16_t last_written_receivedSequence = 0;
uint16_t receivedSeqNum;
    
static void OTA_Send_Ack(void);

static void OTA_Write_Data(void); 

static uint16_t conn_handle; 

/**
 * @brief  It jumps to the new upgraded application
 * @param  None
 * @retval None
 *
 * @note The API code could be subject to change in future releases.
 */
void OTA_Jump_To_New_Application()
{
  /* Reset manager will take care of running the new application */
  NVIC_SystemReset(); 
}

/**
 * @brief  It jumps to the OTA Service Manager application
 * @param  None
 * @retval None
 *
 * @note The API code could be subject to change in future releases.
 */
void OTA_Jump_To_Service_Manager_Application()
{
#ifdef ST_USE_OTA_SERVICE_MANAGER_APPLICATION
  extern volatile uint32_t ota_sw_activation;
  ota_sw_activation = OTA_APP_SWITCH_OP_CODE_GO_TO_OTA_SERVICE_MANAGER; 
  
  NVIC_SystemReset();
#endif
}
  
/**
 * @brief  It just informs OTA manager of disconnection complete event in order to
 *         jump to new application
 * @param  None
 * @retval None
 *
 * @note The API code could be subject to change in future releases.
 */  
void OTA_terminate_connection(void)
{
  bootloadingCompleted_end = ota_allow_jump; 
}

/**
 * @brief  It returns the OTA upgrade fw status
 * @param  None
 * @retval 1 if OTA upgrade session has been completed; 0 otherwise
 *
 * @note The API code could be subject to change in future releases.
 */
uint8_t OTA_Tick()
{
  if (bootloadingCompleted) 
  { 
    bootloadingCompleted = 0; 
    PRINTF("** Over The Air BLE  FW upgrade completed with success! *****************\r\n");
    PRINTF("** Application is JUMPING to new base address: 0x%08X *********************\r\n",(unsigned int)imageBase);
    /*  Turn off radio activity mask */
    aci_hal_set_radio_activity_mask(0x0000);
    /* Terminate connection with option to performs pending operations on stack queue */
    aci_gap_terminate(conn_handle, 0x93);
  }
  
  return (bootloadingCompleted_end);
}

static void OTA_Set_Application_Tag_Value(uint32_t address,uint32_t Data)
{
  FLASH_ProgramWord(address + OTA_TAG_VECTOR_TABLE_ENTRY_OFFSET,Data);
}

/**
 * @brief  It sets the related OTA application
 *         validity tags for handling the proper jumping to the valid application. 
 * @param  None
 * @retval None
 *
 * @note The API code could be subject to change in future releases.
 */
static void OTA_Set_Validity_Tags(void) 
{
  /* Based on the application type, the application validity tag is set */
  if (OTA_OP_CODE == OTA_APP_SWITCH_OP_CODE_GO_TO_LOWER_APP) // Lower Application OTA done with success  
  {
    /* Set valid tag x lower application (the new application just successfully upgraded through OTA) */
    OTA_Set_Application_Tag_Value(APP_LOWER_ADDRESS, OTA_VALID_TAG);
    
    /* Set invalid/old tag for old higher application */
    OTA_Set_Application_Tag_Value(APP_HIGHER_ADDRESS, OTA_INVALID_OLD_TAG); 
  }
  else if (OTA_OP_CODE == OTA_APP_SWITCH_OP_CODE_GO_TO_HIGHER_APP) // Higher Application OTA done with success  
  {
    /* Set valid tag x higher application (the new application just successfully upgraded through OTA) */
    OTA_Set_Application_Tag_Value(APP_HIGHER_ADDRESS, OTA_VALID_TAG);
    
    /* Set invalid/old tag for old lower application */
    OTA_Set_Application_Tag_Value(APP_LOWER_ADDRESS, OTA_INVALID_OLD_TAG); 
  }
  else if (OTA_OP_CODE == OTA_APP_SWITCH_OP_CODE_GO_TO_NEW_APP) // OTA Service Manager has upgraded a new application with success  
  {
    /* Set valid tag x the new application just successfully upgraded through OTA */
    OTA_Set_Application_Tag_Value(APP_WITH_OTA_SERVICE_ADDRESS, OTA_VALID_TAG);
  }
  
}


/**
 * @brief  Verifies flash content.
 * @param  currentWriteAddress: beginning of written address
 *         pbuffer: target buffer address
 *         size: buffer size
 * @retval Status.
 *
 * @note The API code could be subject to change in future releases.
 */
static ErrorStatus FLASH_Verify(uint32_t currentWriteAddress,uint32_t * pbuffer,uint8_t size)
{
  uint8_t * psource = (uint8_t*) (currentWriteAddress);
  uint8_t * pdest   = (uint8_t*) pbuffer;
  
  for(;(size>0) && (*(psource++) == *(pdest++)) ;size--);
  
  if (size>0)
    return ERROR;
  else
    return SUCCESS;
}

/**
 * @brief  Init OTA
 * @param  None.
 * @retval Value indicating success or error code.
 *
 * @note The API code could be subject to change in future releases.
 */
static void OTA_Init(void)
{
  SdkEvalLedInit(OTA_LED); //bootloader is ongoing led
}

/**
 * @brief  Add the 'OTABootloader' service.
 * @param  None.
 * @retval Value indicating success or error code.
 *
 * @note The API code could be subject to change in future releases.
 */
tBleStatus OTA_Add_Btl_Service(void)
{
    tBleStatus ret;
    uint8_t uuid[16];
    
    OTA_Init();
    COPY_BTL_SERVICE_UUID(uuid);
    Osal_MemCpy(&ota_service_uuid.Service_UUID_128, uuid, 16);
    /* OTA BOOTLOADER SERVICE ALLOCATION */
    ret = aci_gatt_add_service(UUID_TYPE_128,  &ota_service_uuid, PRIMARY_SERVICE, 10, &btlServHandle);
    if (ret != BLE_STATUS_SUCCESS) goto fail;    
    
    COPY_IMAGE_CHAR_UUID(uuid);
    Osal_MemCpy(&ota_char_uuid.Char_UUID_128, uuid, 16);
    /* 1ST OTA SERVICE CHARAC ALLOCATION: FREE FLASH RANGE INFOS */
    ret =  aci_gatt_add_char(btlServHandle, UUID_TYPE_128, &ota_char_uuid, 8, CHAR_PROP_READ, ATTR_PERMISSION_NONE, GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
                             16, 0, &btlImageCharHandle);
    if (ret != BLE_STATUS_SUCCESS) goto fail;

#ifdef ST_OTA_BTL_ECHO    
    PRINTF("\r\n");
    PRINTF("\r\n *** btlImageCharHandle : 0x%04X ***\r\n", btlImageCharHandle);
    PRINTF("\r\n");
#endif
    
    COPY_NEW_IMAGE_CHAR_UUID(uuid);
    Osal_MemCpy(&ota_char_uuid.Char_UUID_128, uuid, 16);
    /* 2ND OTA SERVICE CHARAC ALLOCATION: BASE AND SIZE OF NEW IMAGE + NOTIFICATION WINDOW */ 
    ret =  aci_gatt_add_char(btlServHandle, UUID_TYPE_128, &ota_char_uuid, 9, CHAR_PROP_READ|CHAR_PROP_WRITE|CHAR_PROP_WRITE_WITHOUT_RESP, ATTR_PERMISSION_NONE, GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP | GATT_NOTIFY_ATTRIBUTE_WRITE,
                             16, 0, &btlNewImageCharHandle);
    if (ret != BLE_STATUS_SUCCESS) goto fail;

#ifdef ST_OTA_BTL_ECHO     
    PRINTF("\r\n");
    PRINTF("\r\n *** btlNewImageCharHandle : 0x%04X ***\r\n", btlNewImageCharHandle);
    PRINTF("\r\n");
#endif
    
    COPY_NEW_IMAGE_TU_CONTENT_CHAR_UUID(uuid);
    Osal_MemCpy(&ota_char_uuid.Char_UUID_128, uuid, 16);
    /* 3RD OTA SERVICE CHARAC ALLOCATION: IMAGE CONTENT  */
    ret =  aci_gatt_add_char(btlServHandle, UUID_TYPE_128, &ota_char_uuid, 20, CHAR_PROP_READ|CHAR_PROP_WRITE|CHAR_PROP_WRITE_WITHOUT_RESP, ATTR_PERMISSION_NONE, GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP | GATT_NOTIFY_ATTRIBUTE_WRITE,
                             16, 0, &btlNewImageTUContentCharHandle);
    if (ret != BLE_STATUS_SUCCESS) goto fail;

#ifdef ST_OTA_BTL_ECHO     
    PRINTF("\r\n");
    PRINTF("\r\n *** btlNewImageTUContentCharHandle : 0x%04X ***\r\n", btlNewImageTUContentCharHandle);
    PRINTF("\r\n");
#endif    

    COPY_EXPECTED_IMAGE_TU_SEQNUM_CHAR_UUID(uuid);
    Osal_MemCpy(&ota_char_uuid.Char_UUID_128, uuid, 16);
    /* 4TH OTA SERVICE CHARAC ALLOCATION: IMAGE BLOCK FOR NOTIFICATION, INCLUDES NEXT EXPECTED IMAGE NUMBER AND ERROR CONDITIONS */
    ret =  aci_gatt_add_char(btlServHandle, UUID_TYPE_128, &ota_char_uuid, 4, CHAR_PROP_NOTIFY|CHAR_PROP_READ, ATTR_PERMISSION_NONE, GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
                             16, 0, &btlExpectedImageTUSeqNumberCharHandle);
    if (ret != BLE_STATUS_SUCCESS) goto fail;

#ifdef ST_OTA_BTL_ECHO     
    PRINTF("\r\n");
    PRINTF("\r\n *** btlExpectedImageTUSeqNumberCharHandle : 0x%04X ***\r\n", btlExpectedImageTUSeqNumberCharHandle);
    PRINTF("\r\n");
#endif
    /* Free flash space advertised by the application (data for 1ST OTA BTL characteristic initialization) */
    uint32_t currentImageInfos[2];
    currentImageInfos[0] = OTA_FREE_SPACE_RANGE_START;
    currentImageInfos[1] = OTA_FREE_SPACE_RANGE_END;

    /* We provide defaults current image information characteristic (1ST CHAR)*/
    ret = aci_gatt_update_char_value(btlServHandle, btlImageCharHandle, 0, 8,(uint8_t *) currentImageInfos);
    if (ret != BLE_STATUS_SUCCESS)
    {
      PRINTF("Error while updating characteristic.\n");
      return BLE_STATUS_ERROR;
    }
    
    return BLE_STATUS_SUCCESS;
           
fail:
    return BLE_STATUS_ERROR ;
    
}/* end OTA_Add_Btl_Service() */


static void OTA_Check_Update_Error_Condition(uint8_t notification_range)
{
  tBleStatus ret;
  
  /* Check if the notification reporting the error condition can be sent  
     (notification are sent inline with the expected notification window */
  if (((write_counter+1) % notification_range) == 0) 
  {              
     ret = aci_gatt_update_char_value(btlServHandle, btlExpectedImageTUSeqNumberCharHandle, 0, 4,(uint8_t*)&notification);
     if (ret != BLE_STATUS_SUCCESS)
       PRINTF("Error while updating  characteristic.\n");
    
     detected_error = 0;
     write_counter = 0; 
  } 
}

static void OTA_Set_Error_Flags(uint8_t error_condition, uint16_t expectedSeqNum)
{
  /* Set error flag */
   detected_error = 1;
   
   /* Set checksum error with expected sequence number */
   notification.errCode = error_condition;
   notification.replyCounter = expectedSeqNum;
}

/* it sends the ack to OTA client */ 
void OTA_Send_Ack(void)
{
   tBleStatus ret;
   
  /* Depending on outcome of code section above send notification related to: 
  * next sequence number *OR* flash write failure *OR* verify failure 
  */
  ret = aci_gatt_update_char_value(btlServHandle, btlExpectedImageTUSeqNumberCharHandle, 0, 4,(uint8_t*)&notification);
  if (ret != BLE_STATUS_SUCCESS)
  {
    PRINTF("Error while updating btlExpectedImageTUSeqNumberCharHandle characteristic.\n");
  }
  if (totalBytesWritten >= imageSize)
  { 

   /* light down led on the BlueNRG-1 platform to advertise beginning of OTA bootloading session */
   SdkEvalLedOff(OTA_LED);

   /* Set the validity tags for the new app and old one */
   OTA_Set_Validity_Tags();
   /* set flag for ota fw upgrade process completed */
   bootloadingCompleted = 1;
   /* jump to new application is allowed */
   ota_allow_jump = 1; 
  }
}/* end OTA_Send_Ack() */

/* It writes the received data into OTA slave flash */
void OTA_Write_Data(void)
{
  uint8_t verifyStatus;
 
  uint16_t k;
   
  ota_write_data = 0; 
  
  /* store the potential last sequence number successfully written (if no flash write errors are detected) */
  last_written_receivedSequence = receivedSeqNum; 

  /* drop buffer into flash if it's the right time */
  currentWriteAddress = imageBase + totalBytesWritten;
  k=0;

  verifyStatus = SUCCESS;
  while (k<bufPointer)
  {
    /* don't change the OTA validity tag value during OTA upgrade session: it stays to 0xFFFFFFFF */
    if (currentWriteAddress != (imageBase + OTA_TAG_VECTOR_TABLE_ENTRY_OFFSET))
    {
       uint8_t  byteIncrement = BYTE_INCREMENT; /* Flash Burst Write: 4 words (16 bytes) */
        
       if ((bufPointer - k) >= BYTE_INCREMENT)
       {
          FLASH_ProgramWordBurst(currentWriteAddress, (uint32_t *)(&imageBuffer[k]));
          verifyStatus = FLASH_Verify(currentWriteAddress,(uint32_t *)(&imageBuffer[k]),BYTE_INCREMENT);
       } 
       else 
       {
         uint8_t  byteIncrement = 4; /* Flash Write: 1 word (4 bytes) */
         FLASH_ProgramWord(currentWriteAddress, (((uint32_t)imageBuffer[k+3]<< 24) + ((uint32_t)imageBuffer[k+2]<< 16) + ((uint32_t)imageBuffer[k+1]<< 8) + (uint32_t)imageBuffer[k]) );           
         verifyStatus = FLASH_Verify(currentWriteAddress,(uint32_t *)(&imageBuffer[k]),byteIncrement);
       }
       
       if (verifyStatus == SUCCESS)
       {  
          k+=byteIncrement;
          currentWriteAddress += byteIncrement;
       } 
       else
          break;
    }
    else
    {  
       k+=BYTE_INCREMENT; /* skip bytes write related to OTA validity tag */ 
       currentWriteAddress += BYTE_INCREMENT;
    }
  }/* end while */
  /* prepare notification data for next expected block if for both success or notify write/verify failure */
  if (verifyStatus == SUCCESS) 
  {
    totalBytesWritten+=bufPointer;
    notification.errCode = OTA_SUCCESS;
    /* reser buffer pointer, everything was successfully written on flash */
    bufPointer = 0; 
  } 
  else 
  {
  #ifdef ST_OTA_BTL_MINIMAL_ECHO
    PRINTF("Flash verify failure \r\n");
  #endif
    notification.errCode = OTA_FLASH_VERIFY_ERROR;                
  }
 
  OTA_Send_Ack();
  
}/* end OTA_Write_Data() */


/** 
 * @brief This function handles the OTA bootloader updgrade. 
 * It is called on the aci_gatt_attribute_modified_event() callback context for handling the
 * the specific characteristic wirte coming from the OTA Client.
 * 
 * @param Connection_Handle Handle of the connection.
 * @param attr_handle Handle of the OTA attribute that was modified.
 * @param data_length Length of att_data in octets
 * @param att_data    The modified value
 *
 * @retval None
 *
 * @note The API code could be subject to change in future releases.
 */
void OTA_Write_Request_CB(uint16_t connection_handle, 
                          uint16_t attr_handle,
                          uint8_t data_length,
                          uint8_t *att_data)
{
    tBleStatus ret;
    uint16_t k;
    
    conn_handle = connection_handle;
    
    if (attr_handle == (btlNewImageCharHandle + 1)){
      
      erase_flash_done = 0;
      ota_service_is_disconnected=0;
      
      /* Incoming write charachteristic to allow master to specify the base address and size
       * of the firmware image it intends to send. 
       * Get base_address and image size. 
       */
      imageSize = (uint32_t)(att_data[4] << 24) + (uint32_t)(att_data[3] << 16) + (uint32_t)(att_data[2] << 8) + att_data[1];
      imageBase = (uint32_t)(att_data[8] << 24) + (uint32_t)(att_data[7] << 16) + (uint32_t)(att_data[6] << 8) + att_data[5];
      notification_range = NOTIFICATION_INTERVAL(att_data[0]); 

      currentWriteAddress = imageBase;
      numPages = imageSize/PAGE_SIZE; 
      bufPointer = 0;
      totalBytesWritten = 0;
      expectedSeqNum = 0;
#ifdef ST_OTA_BTL_MINIMAL_ECHO
      PRINTF("Free Image base = 0x%08X ; Image size = 0x%08X, numPages = %d\r\n",(unsigned int)imageBase,(unsigned int)imageSize,numPages+1);
#endif  
      
      /*  0x0004: "Connection event slave" */
      ret = aci_hal_set_radio_activity_mask(0x0004);
      if(ret != BLE_STATUS_SUCCESS) {
        PRINTF("aci_hal_set_radio_activity_mask()failed: 0x%02x\r\n", ret);
      }
      
    } else  if (attr_handle == (btlExpectedImageTUSeqNumberCharHandle + 2)){

     /* Here we are handling write characteristic descriptor, switch on notifications.
      * At this point it performs required pages erase according to the previously provided image 
      * size and provide notification
      */
      expectedSeqNum = 0;

      notification.replyCounter = 0;
      notification.errCode = 0;
      ret = aci_gatt_update_char_value(btlServHandle, btlExpectedImageTUSeqNumberCharHandle, 0, 4,(uint8_t*)&notification);
      if (ret != BLE_STATUS_SUCCESS) 
      {
        PRINTF("Error while updating btlExpectedImageTUSeqNumberCharHandle characteristic.\n");
      } 
      else 
      {  
        /* light up led on the BlueNRG platform to advertise beginning of OTA bootloading session */
        SdkEvalLedOn(OTA_LED);
        
        /* warn beginning of bootloading session through gloabal variable */
        //bootloadingOngoing = 1;
      }
   }
    else if (attr_handle == (btlNewImageTUContentCharHandle + 1))
    {
       /* Check if a checksum or sequence number error has been detected */
       if (detected_error) 
       {
         /* An error has been detected: just count the coming next write until the end of current
            notification window: all the writes on this window must be repeated */
         write_counter += 1; 
         /* When the next expected notification from OTA client must be sent sent (inline with notification window),
            the detected error code is notified with the sequence number to be used for retrying again all the writes of this block */
         OTA_Check_Update_Error_Condition(notification_range);
       }
       /* Here we read updated characteristic content filled by "write with no response command' coming from the master */
       else if (bufPointer < BUF_SIZE){
          /* Data will be received by the OTA slave 16 byte wise (due to characteristic image content = 16 bytes image + 4 of headers)
           * Drop new image data into buffer
           */
         
          for(k=bufPointer; k<(bufPointer + data_length - 4); k++){ // Store 16 bytes of received notification on imageBuffer 
             if (k<imageSize)
               imageBuffer[k] = att_data[(k - bufPointer) + 1];
             else
               /* zero pad unutilized residual*/
               imageBuffer[k] = 0;
             
             checksum ^= imageBuffer[k];
          }
          /* include header data into checksum processing as well */
          checksum ^= (att_data[data_length - 3] ^ att_data[data_length - 2] ^ att_data[data_length - 1]);          
          bufPointer = k;
          /* In the section of code below: notify for received packet integrity (cheksum), sequence number correctness
           * and eventually write flash (which will get notified as well)
          */
          
          /* check checksum */ 
          if (checksum == att_data[0]){
             /* checksum ok */      
             /* sequence number check */
             receivedSeqNum = ((att_data[19]<<8) + att_data[18]);
             if (expectedSeqNum == receivedSeqNum) 
             { 
               /* sequence number check ok, increment expected sequence number and prepare for next block notification */
               expectedSeqNum++;
               
               if ((((receivedSeqNum+1) % notification_range) == 0) || (((receivedSeqNum+1)*16) >= imageSize)) 
               { 
                 ota_do_notification = 0; 
                 /* Here is where we manage notifications related to correct sequence number and write/verify
                  * results if conditions get us through the next nested 'if' section (FLASH write section)
                  */
                 /* replyCounter defaults to expectedSeqNum unless flash write fails */
                 notification.replyCounter = expectedSeqNum;
                 notification.errCode = 0x0000;
                
                 if (((bufPointer % BUF_SIZE) == 0) || (((imageSize - totalBytesWritten) < BUF_SIZE)&& (bufPointer >= (imageSize - totalBytesWritten))))
                 {
                    ota_write_data = 1;  
                    ota_do_notification = 1; 
                   
                 }/* end of BUF write management section*/              
                 if (!ota_do_notification) 
                 {
                   /* Depending on outcome of code section above send notification related to: 
                    * next sequence number *OR* flash write failure *OR* verify failure 
                    */
                   ret = aci_gatt_update_char_value(btlServHandle, btlExpectedImageTUSeqNumberCharHandle, 0, 4,(uint8_t*)&notification);
                   if (ret != BLE_STATUS_SUCCESS)
                   {
                      PRINTF("Error while updating btlExpectedImageTUSeqNumberCharHandle characteristic.\n");
                   }
                   if (totalBytesWritten >= imageSize)
                   { 

                     /* light down led on the BlueNRG-1 platform to advertise beginning of OTA bootloading session */
                     SdkEvalLedOff(OTA_LED);
               
                     /* Set the validity tags for the new app and old one */
                     OTA_Set_Validity_Tags();
                     /* set flag for ota fw upgrade process completed */
                     bootloadingCompleted = 1;
                     /* jump to new application is allowed */
                     ota_allow_jump = 1; 
                   }
                 } 
               } /* end of notification window section */
             } 
             else 
             { 
                /* notify sequence number failure */
                write_counter = expectedSeqNum;
                
                /* set new expected sequence number */
                expectedSeqNum = (last_written_receivedSequence == 0) ? last_written_receivedSequence: (last_written_receivedSequence+1); 
                 
                /* Set error flags for sequence number error */
                OTA_Set_Error_Flags(OTA_SEQUENCE_ERROR,expectedSeqNum); 
                 
                /* null packet due to seq failure: remove from internal buffer through buffer pointer shift */
                bufPointer = 0;
#ifdef ST_OTA_BTL_MINIMAL_ECHO
                PRINTF("Sequence number check failed, expected frame # 0x%02X but 0x%02X was received \r\n", expectedSeqNum,receivedSeqNum);      
#endif
                /* An error has been detected just on last write of current notification window: the detected error code can be notified now
                   since OTA client is ready to get the expected notification.
                   Notification is done with the sequence number to be used for retrying again all the writes of this block */
                OTA_Check_Update_Error_Condition(notification_range);
             }            
          } /* if check sum */
          else 
          { 
             /* notify checksum failure */
             write_counter = expectedSeqNum;
            
             /* set new expected sequence number */
             expectedSeqNum = (last_written_receivedSequence == 0) ? last_written_receivedSequence: (last_written_receivedSequence+1); 
             
             /* Set error flags for checksum error*/
             OTA_Set_Error_Flags(OTA_CHECKSUM_ERROR,expectedSeqNum); 

             /* null packet due to seq failure: remove from internal buffer through buffer pointer shift */
             bufPointer = 0; 
            
#ifdef ST_OTA_BTL_MINIMAL_ECHO
             PRINTF("CheckSum error on expected frame # 0x%02X\r\n", expectedSeqNum);            
#endif
             /* An error has been detected just on last write of current notification window: the detected error code can be notified now
                since OTA client is ready to get the expected notification.
                Notification is done with the sequence number to be used for retrying again all the writes of this block */
             OTA_Check_Update_Error_Condition(notification_range);
          }
          checksum = 0;
       }
    }
}/* end OTA_Write_Request_CB() */


/**
 * @brief  It tracks the timing for next radio activity slot when connected as slave
 * @param  Next_State_SysTime time of next radio activity slot 
 * @retval None
 *
 * @note The API code could be subject to change in future releases.
 */
void OTA_Radio_Activity(uint32_t Next_State_SysTime)
{
  /* Check if data for flash write are ready: do write only if there is enough time before next radio activity */
  if (ota_write_data) 
  {
    /* Data buffer are available for Flash write */
    if (HAL_VTimerDiff_ms_sysT32(Next_State_SysTime, HAL_VTimerGetCurrentTime_sysT32()) > OTA_WRITE_GUARD_TIME) 
    {
        ota_write_data = 0; 
        OTA_Write_Data(); 
    }
  }
  
}
/******************* (C) COPYRIGHT 2015 STMicroelectronics *****END OF FILE****/
