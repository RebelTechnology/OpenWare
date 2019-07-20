/**
  ******************************************************************************
  * @file    usbd_audio.c
  * @author  MCD Application Team
  * @version V2.4.2
  * @date    11-December-2015
  * @brief   This file provides the Audio core functions.
  *
  * @note     In HS mode and when the DMA is used, all variables and data structures
  *           dealing with the DMA during the transaction process should be 32-bit aligned.
  *           
  *      
  */ 

#include "usbd_audio.h"
#include "usbd_desc.h"
#include "usbd_ctlreq.h"
#include "device.h"
#include "midi.h"
#include "message.h"


#define AUDIO_SAMPLE_FREQ(frq)  (uint8_t)(frq), (uint8_t)((frq >> 8)), (uint8_t)((frq >> 16))
#define AUDIO_PACKET_SZE(frq)   (uint8_t)(((frq * USB_AUDIO_CHANNELS * AUDIO_BYTES_PER_SAMPLE)/1000) & 0xFF), \
    (uint8_t)((((frq * USB_AUDIO_CHANNELS * AUDIO_BYTES_PER_SAMPLE)/1000) >> 8) & 0xFF)

#define USB_MIDI_CONFIG_DESC_SIZ       0x65
#define USB_AUDIO_CONFIG_DESC_SIZ       0x65

#define MIDI_OUT_EP                    0x01
#define MIDI_IN_EP                     0x81
#define MIDI_DATA_IN_PACKET_SIZE       0x40
#define MIDI_DATA_OUT_PACKET_SIZE      0x40

/* #define MIDI_OUT_EP                    0x02 */
/* #define MIDI_IN_EP                     0x82 */
/* #define MIDI_DATA_IN_PACKET_SIZE       0x40 */
/* #define MIDI_DATA_OUT_PACKET_SIZE      0x40 */

/* #define AUDIO_OUT_EP                                  0x01 */
/* #define AUDIO_IN_EP                                   0x81 */
/* #define USB_AUDIO_CONFIG_DESC_SIZ                     174 */


static uint8_t  USBD_AUDIO_Init (USBD_HandleTypeDef *pdev, 
                               uint8_t cfgidx);

static uint8_t  USBD_AUDIO_DeInit (USBD_HandleTypeDef *pdev, 
                                 uint8_t cfgidx);

static uint8_t  USBD_AUDIO_Setup (USBD_HandleTypeDef *pdev, 
                                USBD_SetupReqTypedef *req);

static uint8_t  *USBD_AUDIO_GetCfgDesc (uint16_t *length);

static uint8_t  *USBD_AUDIO_GetDeviceQualifierDesc (uint16_t *length);

static uint8_t  USBD_AUDIO_DataIn (USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t  USBD_AUDIO_DataOut (USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t  USBD_AUDIO_EP0_RxReady (USBD_HandleTypeDef *pdev);

static uint8_t  USBD_AUDIO_EP0_TxReady (USBD_HandleTypeDef *pdev);

static uint8_t  USBD_AUDIO_SOF (USBD_HandleTypeDef *pdev);

static uint8_t  USBD_AUDIO_IsoINIncomplete (USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t  USBD_AUDIO_IsoOutIncomplete (USBD_HandleTypeDef *pdev, uint8_t epnum);

static void AUDIO_REQ_GetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);

static void AUDIO_REQ_SetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);

USBD_AUDIO_HandleTypeDef usbd_audio_handle CCM;

USBD_ClassTypeDef  USBD_AUDIO =
{
  USBD_AUDIO_Init,
  USBD_AUDIO_DeInit,
  USBD_AUDIO_Setup,
  USBD_AUDIO_EP0_TxReady, 
  USBD_AUDIO_EP0_RxReady,
  USBD_AUDIO_DataIn,
  USBD_AUDIO_DataOut,
  USBD_AUDIO_SOF,
  USBD_AUDIO_IsoINIncomplete,
  USBD_AUDIO_IsoOutIncomplete,     
  USBD_AUDIO_GetCfgDesc,
  USBD_AUDIO_GetCfgDesc,
  USBD_AUDIO_GetCfgDesc,
  USBD_AUDIO_GetDeviceQualifierDesc,
};

/* USB AUDIO device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_AUDIO_CfgDesc[USB_AUDIO_CONFIG_DESC_SIZ] __ALIGN_END =
{
  /* Configuration 1 */
  0x09,                                 /* bLength */
  0x02,                                 /* bDescriptorType */
  LOBYTE(USB_MIDI_CONFIG_DESC_SIZ),       /* wTotalLength */
  HIBYTE(USB_MIDI_CONFIG_DESC_SIZ),       /* wTotalLength */
  0x02,                                 /* bNumInterfaces */
  0x01,                                 /* bConfigurationValue */
  0x00,                                 /* iConfiguration */
  0x80,                                 /* bmAttributes: BUS Powered */
  0x32,                                 /* bMaxPower = 100 mA*/
  /* 09 bytes */
  
  /* Standard AC Interface Descriptor */
  0x09,                                 /* bLength */
  0x04,                                 /* bDescriptorType */
  0x00,                                 /* bInterfaceNumber */
  0x00,                                 /* bAlternateSetting */
  0x00,                                 /* bNumEndpoints */
  0x01,                                 /* bInterfaceClass */
  0x01,                                 /* bInterfaceSubClass */
  0x00,                                 /* bInterfaceProtocol */
  0x00,                                 /* iInterface */
  /* 09 bytes */
  
  /* Class-specific AC Interface Descriptor */
  0x09,                                 /* bLength */
  0x24,                                 /* bDescriptorType */
  0x01,                                 /* bDescriptorSubtype */
  0x00,                                 /* bcdADC */
  0x01,                                 /* bcdADC */
  0x09,                                 /* wTotalLength */
  0x00,					/* wTotalLength */
  0x01,                                 /* bInCollection */
  0x01,                                 /* baInterfaceNr */
  /* 09 bytes */
  
  /* Standard MS Interface Descriptor */
  /* MIDI Adapter Standard MS Interface Descriptor */
  0x09,                                 /* bLength */
  0x04,                                 /* bDescriptorType */
  0x01,                                 /* bInterfaceNumber */
  0x00,                                 /* bAlternateSetting */
  0x02,                                 /* bNumEndpoints */
  0x01,                                 /* bInterfaceClass */
  0x03,                                 /* bInterfaceSubClass */
  0x00,                                 /* bInterfaceProtocol */
  0x00,                                 /* iInterface */
  /* 09 bytes */
  
  /* Class-specific MS Interface Descriptor */
  /* MIDI Adapter Class-specific MS Interface Descriptor */
  0x07,                                 /* bLength */
  0x24,                                 /* bDescriptorType */
  0x01,                                 /* bDescriptorSubtype */
  0x00,                                 /* bcdADC */
  0x01,                                 /* bcdADC */
  0x41,                                 /* wTotalLength */
  0x00,                                 /* wTotalLength */
  /* 07 bytes */
  
  /* MIDI Adapter MIDI IN Jack Descriptor (Embedded) */
  0x06,                                 /* bLength */
  0x24,                                 /* bDescriptorType */
  0x02,                                 /* bDescriptorSubtype */
  0x01,                                 /* bJackType */
  0x01,					/* bJackID */
  0x00,                                 /* iJack */
  /* 06 bytes */
  
  /* MIDI Adapter MIDI IN Jack Descriptor (External) */
  0x06,                                 /* bLength */
  0x24,                                 /* bDescriptorType */
  0x02,                                 /* bDescriptorSubtype */
  0x02,                                 /* bJackType */
  0x02,					/* bJackID */
  0x00,                                 /* iJack */
  /* 06 bytes */

  /* MIDI Adapter MIDI OUT Jack Descriptor (Embedded) */
  0x09,                                 /* bLength */
  0x24,                                 /* bDescriptorType */
  0x03,                                 /* bDescriptorSubtype */
  0x01,                                 /* bJackType */
  0x03,					/* bJackID */
  0x01,                                 /* bNrInputPins */
  0x02,                                 /* BaSourceID */
  0x01,                                 /* BaSourcePin */
  0x00,                                 /* iJack */
  /* 09 bytes */

  /* MIDI Adapter MIDI OUT Jack Descriptor (External) */
  0x09,                                 /* bLength */
  0x24,                                 /* bDescriptorType */
  0x03,                                 /* bDescriptorSubtype */
  0x02,                                 /* bJackType */
  0x04,					/* bJackID */
  0x01,                                 /* bNrInputPins */
  0x01,                                 /* BaSourceID */
  0x01,                                 /* BaSourcePin */
  0x00,                                 /* iJack */
  /* 09 bytes */

  /* MIDI Adapter Standard Bulk OUT Endpoint Descriptor */
  0x09,                                 /* bLength */
  0x05,                                 /* bDescriptorType */
  MIDI_OUT_EP,                          /* bEndpointAddress */
  0x02,                                 /* bmAttributes */
  0x40,					/* wMaxPacketSize */
  0x00,                                 /* wMaxPacketSize */
  0x00,                                 /* bInterval */
  0x00,                                 /* bRefresh */
  0x00,                                 /* bSynchAddress */
  /* 09 bytes */

  /* MIDI Adapter Class-specific Bulk OUT Endpoint Descriptor */
  0x05,                                 /* bLength */
  0x25,                                 /* bDescriptorType */
  0x01,                                 /* bDescriptorSubtype */
  0x01,                                 /* bNumEmbMIDIJack */
  0x01,					/* BaAssocJackID */
  /* 05 bytes */

  /* MIDI Adapter Standard Bulk IN Endpoint Descriptor */
  0x09,                                 /* bLength */
  0x05,                                 /* bDescriptorType */
  MIDI_IN_EP,                           /* bEndpointAddress */
  0x02,                                 /* bmAttributes */
  0x40,					/* wMaxPacketSize */
  0x00,                                 /* wMaxPacketSize */
  0x00,                                 /* bInterval */
  0x00,                                 /* bRefresh */
  0x00,                                 /* bSynchAddress */
  /* 09 bytes */

  /* MIDI Adapter Class-specific Bulk IN Endpoint Descriptor */
  0x05,                                 /* bLength */
  0x25,                                 /* bDescriptorType */
  0x01,                                 /* bDescriptorSubtype */
  0x01,                                 /* bNumEmbMIDIJack */
  0x03					/* BaAssocJackID */
  /* 05 bytes */
} ;

/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_AUDIO_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END=
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};

/**
  * @}
  */ 

/** @defgroup USBD_AUDIO_Private_Functions
  * @{
  */ 

/**
  * @brief  USBD_AUDIO_Init
  *         Initialize the AUDIO interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_AUDIO_Init (USBD_HandleTypeDef *pdev, 
                               uint8_t cfgidx)
{
  USBD_AUDIO_HandleTypeDef   *haudio;

    USBD_StatusTypeDef rv;

#ifdef USE_USBD_AUDIO_OUT
    /* Open OUT (i.e. speaker) Endpoint */
    rv = USBD_LL_OpenEP(pdev,
			AUDIO_OUT_EP,
			USBD_EP_TYPE_ISOC,
			AUDIO_OUT_PACKET);
    if(rv != USBD_OK )
      USBD_ErrLog("Open of OUT streaming endpoint failed. error %d\n", rv);
#endif

#ifdef USE_USBD_AUDIO_IN
  /* Open IN (i.e. microphone) Endpoint */
    rv = USBD_LL_OpenEP(pdev,
			AUDIO_IN_EP,
			USBD_EP_TYPE_ISOC,
			AUDIO_IN_PACKET_SIZE);
    if(rv != USBD_OK ) 
      USBD_ErrLog("Open of IN streaming endpoint failed. error %d\n", rv);
#endif

    /* Assign Audio structure */
    /* pdev->pClassData = USBD_malloc(sizeof (USBD_AUDIO_HandleTypeDef)); */
    pdev->pClassData = &usbd_audio_handle;
  
    haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;
    haudio->alt_setting = 0;
    haudio->offset = AUDIO_OFFSET_UNKNOWN;
    haudio->wr_ptr = 0; 
    haudio->rd_ptr = 0;  
    haudio->rd_enable = 0;
    
    /* Initialize the Audio output Hardware layer */
    /* if (((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->Init(USBD_AUDIO_FREQ, AUDIO_DEFAULT_VOLUME, 0) != USBD_OK) */
    /* { */
    /*   return USBD_FAIL; */
    /* } */
    
#ifdef USE_USBD_AUDIO_OUT
    /* Prepare Out endpoint to receive 1st packet */
    USBD_LL_PrepareReceive(pdev,
                           AUDIO_OUT_EP,
                           haudio->audio_out_buffer,
                           AUDIO_OUT_PACKET);
#endif

#ifdef USE_USBD_MIDI
    /* Open the in EP */
    rv = USBD_LL_OpenEP(pdev,
			MIDI_IN_EP,
			USBD_EP_TYPE_BULK,
			MIDI_DATA_IN_PACKET_SIZE);
    if(rv != USBD_OK ) 
      USBD_ErrLog("Open of IN MIDI endpoint failed. error %d\n", rv);

    /* Open the out EP */
    rv = USBD_LL_OpenEP(pdev,
			MIDI_OUT_EP,
			USBD_EP_TYPE_BULK,
			MIDI_DATA_OUT_PACKET_SIZE);			
    if(rv != USBD_OK ) 
      USBD_ErrLog("Open of OUT MIDI endpoint failed. error %d\n", rv);

    /* Prepare Out endpoint to receive next packet */
    USBD_LL_PrepareReceive(pdev,
			   MIDI_OUT_EP,
			   haudio->midi_in_buffer,
			   MIDI_DATA_OUT_PACKET_SIZE);

#endif
    
    /* I'm not sure why this needs to be here, but if it is not, USBD_AUDIO_DataIn
     * doesn't get called */
    /* ringbuffer.read(mic_data, AUDIO_IN_PACKET_SIZE, true); */
    /* ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->AudioCmd(haudio->buffer, */
    /* 							 AUDIO_IN_PACKET_SIZE/2, */
    /* 							 AUDIO_CMD_START); */
    usbd_audio_start_callback(pdev, haudio);
    
    return USBD_OK;
}

/**
  * @brief  USBD_AUDIO_Init
  *         DeInitialize the AUDIO layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_AUDIO_DeInit (USBD_HandleTypeDef *pdev, 
                                 uint8_t cfgidx)
{

#ifdef USE_USBD_AUDIO_OUT
  /* Close EP OUT */
  USBD_LL_CloseEP(pdev, AUDIO_OUT_EP);		  
#endif

#ifdef USE_USBD_AUDIO_IN
  /* Close EP IN */
  USBD_LL_CloseEP(pdev, AUDIO_IN_EP);
#endif

#ifdef USE_USBD_MIDI
  USBD_LL_CloseEP(pdev, MIDI_IN_EP);
  USBD_LL_CloseEP(pdev, MIDI_OUT_EP);
#endif
  
  /* DeInit  physical Interface components */
  if(pdev->pClassData != NULL){
    /* ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->DeInit(0); */
    /* USBD_free(pdev->pClassData); */
    pdev->pClassData = NULL;
  }

  return USBD_OK;
}

/**
  * @brief  USBD_AUDIO_Setup
  *         Handle the AUDIO specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t  USBD_AUDIO_Setup (USBD_HandleTypeDef *pdev, 
                                USBD_SetupReqTypedef *req)
{
  USBD_AUDIO_HandleTypeDef   *haudio;
  uint16_t len;
  uint8_t *pbuf;
  uint8_t ret = USBD_OK;
  haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;
  
  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
  case USB_REQ_TYPE_CLASS :  
    switch (req->bRequest)
    {
    case AUDIO_REQ_GET_CUR:
      AUDIO_REQ_GetCurrent(pdev, req);
      break;
      
    case AUDIO_REQ_SET_CUR:
      AUDIO_REQ_SetCurrent(pdev, req);   
      break;
      
    default:
      USBD_CtlError (pdev, req);
      ret = USBD_FAIL; 
    }
    break;
    
  case USB_REQ_TYPE_STANDARD:
    switch (req->bRequest)
    {
    case USB_REQ_GET_DESCRIPTOR:      
      if( (req->wValue >> 8) == AUDIO_DESCRIPTOR_TYPE)
      {
        pbuf = USBD_AUDIO_CfgDesc + 18;
        len = MIN(USB_AUDIO_CONFIG_DESC_SIZ , req->wLength);
        USBD_CtlSendData (pdev, 
                          pbuf,
                          len);
      }
      break;
      
    case USB_REQ_GET_INTERFACE :
      USBD_CtlSendData (pdev,
                        (uint8_t *)&(haudio->alt_setting),
                        1);
      break;
      
    case USB_REQ_SET_INTERFACE :
      if ((uint8_t)(req->wValue) <= USBD_MAX_NUM_INTERFACES)
      {
        haudio->alt_setting = (uint8_t)(req->wValue);
      }
      else
      {
        /* Call the error management function (command will be nacked */
        USBD_CtlError (pdev, req);
      }
      break;      
      
    default:
      USBD_CtlError (pdev, req);
      ret = USBD_FAIL;     
    }
  }
  return ret;
}


/**
  * @brief  USBD_AUDIO_GetCfgDesc 
  *         return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_AUDIO_GetCfgDesc (uint16_t *length)
{
  *length = sizeof (USBD_AUDIO_CfgDesc);
  return USBD_AUDIO_CfgDesc;
}

static volatile int midi_tx_lock = 0;

/**
  * @brief  USBD_AUDIO_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_AUDIO_DataIn (USBD_HandleTypeDef *pdev, 
				   uint8_t epnum)
{
#ifdef USE_USBD_AUDIO_IN
  if(epnum == (AUDIO_IN_EP & ~0x80)){
    USBD_AUDIO_HandleTypeDef *haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;
    usbd_audio_data_in_callback(pdev, haudio);
  }else if(epnum == (MIDI_IN_EP & ~0x80)){
    midi_tx_lock = 0;
  }
#endif
  return USBD_OK;
}

/**
  * @brief  USBD_AUDIO_EP0_RxReady
  *         handle EP0 Rx Ready event
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t  USBD_AUDIO_EP0_RxReady (USBD_HandleTypeDef *pdev)
{
  USBD_AUDIO_HandleTypeDef   *haudio;
  haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;

  if (haudio->control.cmd == AUDIO_REQ_SET_CUR){
    USBD_DbgLog("SET_CUR %d\n", haudio->control.unit);
    debugMessage("SET_CUR");
    if (haudio->control.unit == AUDIO_OUT_STREAMING_CTRL)
    {
      usbd_audio_gain_callback(haudio->control.data[0]);
     /* ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->MuteCtl(haudio->control.data[0]);      */
      haudio->control.cmd = 0;
      haudio->control.len = 0;
    }
    else if(haudio->control.unit == AUDIO_IN_STREAMING_CTRL)
    {
      usbd_audio_gain_callback(haudio->control.data[0]);
     /* ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->MuteCtl(haudio->control.data[0]);      */
      haudio->control.cmd = 0;
      haudio->control.len = 0;
    }
  }else if (haudio->control.cmd == AUDIO_REQ_GET_CUR){
    debugMessage("GET_CUR");
    USBD_DbgLog("GET_CUR %d\n", haudio->control.unit);
  }else{
    debugMessage("Control CMD");
    USBD_DbgLog("Control CMD %d\n", haudio->control.cmd);
  }

  return USBD_OK;
}
/**
  * @brief  USBD_AUDIO_EP0_TxReady
  *         handle EP0 TRx Ready event
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t  USBD_AUDIO_EP0_TxReady (USBD_HandleTypeDef *pdev)
{
  USBD_AUDIO_HandleTypeDef   *haudio;
  haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;
  debugMessage("EP0 TxReady");
  if (haudio->control.cmd == AUDIO_REQ_SET_CUR)
  {/* In this driver, to simplify code, only SET_CUR request is managed */
    USBD_DbgLog("SET_CUR %d\n", haudio->control.unit);
    if (haudio->control.unit == AUDIO_OUT_STREAMING_CTRL)
    {
      usbd_audio_gain_callback(haudio->control.data[0]);
     /* ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->MuteCtl(haudio->control.data[0]); */
      haudio->control.cmd = 0;
      haudio->control.len = 0;
    }
  }
  return USBD_OK;
}
/**
  * @brief  USBD_AUDIO_SOF
  *         handle SOF event
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t  USBD_AUDIO_SOF (USBD_HandleTypeDef *pdev)
{
  return USBD_OK;
}

/**
  * @brief  USBD_AUDIO_Sync
  *         handle Sync event
  * @param  pdev: device instance
  * @retval status
  */
void  USBD_AUDIO_Sync (USBD_HandleTypeDef *pdev, AUDIO_OffsetTypeDef offset)
{
  int8_t shift = 0;
  USBD_AUDIO_HandleTypeDef   *haudio;
  haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;
  
  haudio->offset =  offset; 
  
  
  if(haudio->rd_enable == 1)
  {
    haudio->rd_ptr += AUDIO_TOTAL_BUF_SIZE/2;
    
    if (haudio->rd_ptr == AUDIO_TOTAL_BUF_SIZE)
    {
      /* roll back */
      haudio->rd_ptr = 0;
    }
  }
  
  if(haudio->rd_ptr > haudio->wr_ptr)
  {
    if((haudio->rd_ptr - haudio->wr_ptr) < AUDIO_OUT_PACKET)
    {
      shift = -4;
    }
    else if((haudio->rd_ptr - haudio->wr_ptr) > (AUDIO_TOTAL_BUF_SIZE - AUDIO_OUT_PACKET))
    {
      shift = 4;
    }    

  }
  else
  {
    if((haudio->wr_ptr - haudio->rd_ptr) < AUDIO_OUT_PACKET)
    {
      shift = 4;
    }
    else if((haudio->wr_ptr - haudio->rd_ptr) > (AUDIO_TOTAL_BUF_SIZE - AUDIO_OUT_PACKET))
    {
      shift = -4;
    }  
  }

  if(haudio->offset == AUDIO_OFFSET_FULL)
  {
    /* ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->AudioCmd(&haudio->buffer[0], */
    /*                                                      AUDIO_TOTAL_BUF_SIZE/2 - shift, */
    /*                                                      AUDIO_CMD_PLAY);  */
    debugMessage("usb audio offset full");
    /* debugMessage(msg_itoa(shift, 10)); */
    usbd_audio_data_in_callback(pdev, haudio);
    haudio->offset = AUDIO_OFFSET_NONE;           
  }
}

/**
  * @brief  USBD_AUDIO_IsoINIncomplete
  *         handle data ISO IN Incomplete event
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_AUDIO_IsoINIncomplete (USBD_HandleTypeDef *pdev, uint8_t epnum)
{

  return USBD_OK;
}
/**
  * @brief  USBD_AUDIO_IsoOutIncomplete
  *         handle data ISO OUT Incomplete event
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_AUDIO_IsoOutIncomplete (USBD_HandleTypeDef *pdev, uint8_t epnum)
{

  return USBD_OK;
}
/**
  * @brief  USBD_AUDIO_DataOut
  *         handle data OUT Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_AUDIO_DataOut (USBD_HandleTypeDef *pdev, 
				    uint8_t epnum)
{
  USBD_AUDIO_HandleTypeDef   *haudio;
  haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;
  
#ifdef USE_USBD_AUDIO_IN
  if (epnum == AUDIO_OUT_EP){
    /* Increment the Buffer pointer or roll it back when all buffers are full */
    haudio->wr_ptr += AUDIO_OUT_PACKET;
    if (haudio->wr_ptr == AUDIO_TOTAL_BUF_SIZE){
      /* All buffers are full: roll back */
      haudio->wr_ptr = 0;
      if(haudio->offset == AUDIO_OFFSET_UNKNOWN){
        /* ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->AudioCmd(&haudio->buffer[0], */
        /*                                                      AUDIO_TOTAL_BUF_SIZE/2, */
        /*                                                      AUDIO_CMD_START); */
	usbd_audio_start_callback(pdev, haudio);
	haudio->offset = AUDIO_OFFSET_NONE;
      }
    }
    if(haudio->rd_enable == 0){
      if (haudio->wr_ptr == (AUDIO_TOTAL_BUF_SIZE / 2))
        haudio->rd_enable = 1; 
    }
    /* Prepare Out endpoint to receive next audio packet */
    USBD_LL_PrepareReceive(pdev,
                           AUDIO_OUT_EP,
                           &haudio->audio_out_buffer[haudio->wr_ptr], 
                           AUDIO_OUT_PACKET);
  }
#endif /* USE_USBD_AUDIO_IN */

#ifdef USE_USBD_MIDI
  if(epnum == MIDI_OUT_EP){
    /* Forward data to midi callback */
    uint32_t len = USBD_LL_GetRxDataSize(pdev, epnum);
    midi_device_rx(haudio->midi_in_buffer, len);
    /* Prepare Out endpoint to receive next packet */
    USBD_LL_PrepareReceive(pdev,
			   MIDI_OUT_EP,
			   haudio->midi_in_buffer,
			   MIDI_DATA_OUT_PACKET_SIZE);
#endif
  }
  
  return USBD_OK;
}

/**
  * @brief  AUDIO_Req_GetCurrent
  *         Handles the GET_CUR Audio control request.
  * @param  pdev: instance
  * @param  req: setup class request
  * @retval status
  */
static void AUDIO_REQ_GetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{  
  USBD_AUDIO_HandleTypeDef   *haudio;
  haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;
  
  memset(haudio->control.data, 0, 64);
  /* Send the current mute state */
  USBD_CtlSendData (pdev, 
                    haudio->control.data,
                    req->wLength);
}

/**
  * @brief  AUDIO_Req_SetCurrent
  *         Handles the SET_CUR Audio control request.
  * @param  pdev: instance
  * @param  req: setup class request
  * @retval status
  */
static void AUDIO_REQ_SetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{ 
  USBD_AUDIO_HandleTypeDef   *haudio;
  haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;
  
  if (req->wLength)
  {
    /* Prepare the reception of the buffer over EP0 */
    USBD_CtlPrepareRx (pdev,
                       haudio->control.data,                                  
                       req->wLength);    
    
    haudio->control.cmd = AUDIO_REQ_SET_CUR;     /* Set the request value */
    haudio->control.len = req->wLength;          /* Set the request data length */
    haudio->control.unit = HIBYTE(req->wIndex);  /* Set the request target unit */
  }
}


/**
* @brief  DeviceQualifierDescriptor 
*         return Device Qualifier descriptor
* @param  length : pointer data length
* @retval pointer to descriptor buffer
*/
static uint8_t  *USBD_AUDIO_GetDeviceQualifierDesc (uint16_t *length)
{
  *length = sizeof (USBD_AUDIO_DeviceQualifierDesc);
  return USBD_AUDIO_DeviceQualifierDesc;
}

/**
* @brief  USBD_AUDIO_RegisterInterface
* @param  fops: Audio interface callback
* @retval status
*/
uint8_t  USBD_AUDIO_RegisterInterface  (USBD_HandleTypeDef   *pdev, 
                                        USBD_AUDIO_ItfTypeDef *fops)
{
  if(fops != NULL)
  {
    pdev->pUserData= fops;
  }
  return 0;
}

void usbd_audio_write(USBD_HandleTypeDef* pdev, uint8_t* buf, uint32_t len) {
#ifdef USE_USBD_AUDIO_IN
  if(pdev->dev_state == USBD_STATE_CONFIGURED){
    USBD_LL_Transmit(pdev, AUDIO_IN_EP, buf, len);
  }
#endif
}

#ifdef USE_USBD_MIDI
#ifdef USE_USBD_HS
void midi_device_tx(uint8_t* buf, uint32_t len) {
  extern USBD_HandleTypeDef hUsbDeviceHS;
  if(hUsbDeviceHS.dev_state == USBD_STATE_CONFIGURED){
    /* while(midi_tx_lock); */
    midi_tx_lock = 1;
    USBD_LL_Transmit(&hUsbDeviceHS, MIDI_IN_EP, buf, len);
  }
}
uint8_t midi_device_connected(void){
  extern USBD_HandleTypeDef hUsbDeviceHS;
  return hUsbDeviceHS.dev_state == USBD_STATE_CONFIGURED;
}
#endif /* USE_USBD_HS */

#ifdef USE_USBD_FS
void midi_device_tx(uint8_t* buf, uint32_t len) {
  extern USBD_HandleTypeDef hUsbDeviceFS;
  if(hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED){
    /* while(midi_tx_lock); */
    midi_tx_lock = 1;
    USBD_LL_Transmit(&hUsbDeviceFS, MIDI_IN_EP, buf, len);
  }
}
uint8_t midi_device_connected(void){
  extern USBD_HandleTypeDef hUsbDeviceFS;
  return hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED;
}
#endif /* USE_USBD_FS */
#endif /* USE_USBD_MIDI */

uint8_t midi_device_ready(void){
  return midi_tx_lock == 0;
}
