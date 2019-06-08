/**
  ******************************************************************************
  * @file    usbd_audio.c
  * @author  MCD Application Team
  * @version V2.4.2
  * @date    11-December-2015
  * @brief   This file provides the Audio core functions.
  *
  * @verbatim
  *      
  *          ===================================================================      
  *                                AUDIO Class  Description
  *          ===================================================================
 *           This driver manages the Audio Class 1.0 following the "USB Device Class Definition for
  *           Audio Devices V1.0 Mar 18, 98".
  *           This driver implements the following aspects of the specification:
  *             - Device descriptor management
  *             - Configuration descriptor management
  *             - Standard AC Interface Descriptor management
  *             - 1 Audio Streaming Interface (with single channel, PCM, Stereo mode)
  *             - 1 Audio Streaming Endpoint
  *             - 1 Audio Terminal Input (1 channel)
  *             - Audio Class-Specific AC Interfaces
  *             - Audio Class-Specific AS Interfaces
  *             - AudioControl Requests: only SET_CUR and GET_CUR requests are supported (for Mute)
  *             - Audio Feature Unit (limited to Mute control)
  *             - Audio Synchronization type: Asynchronous
  *             - Single fixed audio sampling rate (configurable in usbd_conf.h file)
  *          The current audio class version supports the following audio features:
  *             - Pulse Coded Modulation (PCM) format
  *             - sampling rate: 48KHz. 
  *             - Bit resolution: 16
  *             - Number of channels: 2
  *             - No volume control
  *             - Mute/Unmute capability
  *             - Asynchronous Endpoints 
  *          
  * @note     In HS mode and when the DMA is used, all variables and data structures
  *           dealing with the DMA during the transaction process should be 32-bit aligned.
  *           
  *      
  *  @endverbatim
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "usbd_audio.h"
#include "usbd_desc.h"
#include "usbd_ctlreq.h"
#include "device.h"


/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_AUDIO 
  * @brief usbd core module
  * @{
  */ 

/** @defgroup USBD_AUDIO_Private_TypesDefinitions
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup USBD_AUDIO_Private_Defines
  * @{
  */ 

/**
  * @}
  */ 


/** @defgroup USBD_AUDIO_Private_Macros
  * @{
  */
#define AUDIO_SAMPLE_FREQ(frq)      (uint8_t)(frq), (uint8_t)((frq >> 8)), (uint8_t)((frq >> 16))

#define AUDIO_PACKET_SZE(frq)          (uint8_t)(((frq * USB_AUDIO_CHANNELS * AUDIO_BYTES_PER_SAMPLE)/1000) & 0xFF), \
                                       (uint8_t)((((frq * USB_AUDIO_CHANNELS * AUDIO_BYTES_PER_SAMPLE)/1000) >> 8) & 0xFF)
                                         
/**
  * @}
  */ 




/** @defgroup USBD_AUDIO_Private_FunctionPrototypes
  * @{
  */


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

/**
  * @}
  */ 

/** @defgroup USBD_AUDIO_Private_Variables
  * @{
  */ 

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
  /////////////////////// MICROPHONE //////////////////////////
  /* USB Microphone Configuration Descriptor */
  0x09,//sizeof(USB_CFG_DSC),    // Size of this descriptor in bytes
  USB_DESC_TYPE_CONFIGURATION,                // CONFIGURATION descriptor type (0x02)
  LOBYTE(USB_AUDIO_CONFIG_DESC_SIZ),       /* wTotalLength  109 bytes*/
  HIBYTE(USB_AUDIO_CONFIG_DESC_SIZ),
  2,                      // bNumInterfaces Number of interfaces in this cfg
  1,                      // bConfigurationValue Index value of this configuration
  0,                      // iConfiguration Configuration string index
  0x80,                   // bmAttributes, see usb_device.h
  50,                     // Max power consumption (2X mA) */
  /* 0xC0,                     /\* bmAttributes  BUS Powred*\/ */
  /* 0x32,                     // Max power consumption (100mA) */
 
  /* USB Microphone Standard AC Interface Descriptor  */
  0x09,//sizeof(USB_INTF_DSC),   // Size of this descriptor in bytes
  USB_DESC_TYPE_INTERFACE, // INTERFACE descriptor type
  0x00,                          // bInterfaceNumber Interface Number
  0x00,                          // bAlternateSetting Alternate Setting Number
  0x00,                          // bNumEndpoints Number of endpoints in this i/f
  USB_DEVICE_CLASS_AUDIO,        // bInterfaceClass Class code 0x01
  AUDIO_SUBCLASS_AUDIOCONTROL,   // bInterfaceSubclass Subclass code 0x01
  0x00,                          // bInterfaceProtocol Protocol code
  0x00,                          // iInterface Interface string index 
 
  /* USB Microphone Class-specific AC Interface Descriptor  (CODE == 9)*/
  0x09,                         // Size of this descriptor, in bytes.
  AUDIO_INTERFACE_DESCRIPTOR_TYPE, // bDescriptorType CS_INTERFACE Descriptor Type 0x24
  AUDIO_CONTROL_HEADER,         // bDescriptorSubtype HEADER descriptor subtype 0x01
  0x00,0x01,                    // bcdADC Audio Device compliant to the USB Audio specification version 1.00
  0x1E,0x00,                    // wTotalLength Total number of bytes returned for the class-specific AudioControl interface descriptor.
  // Includes the combined length of this descriptor header and all Unit and Terminal descriptors.
  0x01,                         // bInCollection The number of AudioStreaming interfaces in the Audio Interface Collection to which this AudioControl interface belongs bInCollection
  0x01,                         // baInterfaceNr AudioStreaming interface 1 belongs to this AudioControl interface. baInterfaceNr
  
  /*USB Microphone Input Terminal Descriptor */
  0x0C,                         // Size of the descriptor, in bytes
  AUDIO_INTERFACE_DESCRIPTOR_TYPE, // bDescriptorType CS_INTERFACE Descriptor Type
  AUDIO_CONTROL_INPUT_TERMINAL,    // bDescriptorSubtype INPUT_TERMINAL descriptor subtype
  0x01,                         // bTerminalID ID of this Terminal.
  0x01,0x02,                    // wTerminalType Terminal is Microphone (0x01,0x02) wTerminalType
  0x00,                         // bAssocTerminal No association
  USB_AUDIO_CHANNELS,               // bNrChannels 
  0x00,0x00,                    // wChannelConfig Mono sets no position bits 
  0x00,                         // iChannelNames Unused
  0x00,                         // iTerminal Unused

  /* USB Microphone Output Terminal Descriptor */
  0x09,                            // Size of the descriptor, in bytes (bLength)
  AUDIO_INTERFACE_DESCRIPTOR_TYPE, // CS_INTERFACE Descriptor Type (bDescriptorType)
  AUDIO_CONTROL_OUTPUT_TERMINAL,   // OUTPUT_TERMINAL descriptor subtype (bDescriptorSubtype)
  0x02,                            // ID of this Terminal. (bTerminalID)
  0x01, 0x01,                      // USB Streaming. wTerminalType
  0x00,                            // unused         (bAssocTerminal)
  0x01,                            // From Input Terminal.(bSourceID)
  0x00,                            // unused  (iTerminal)
	  
  /* USB Microphone Audio Feature Unit Descriptor */
  /* 0x0c,                                 /\* bLength *\/ */
  /* AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /\* bDescriptorType *\/ */
  /* AUDIO_CONTROL_FEATURE_UNIT,           /\* bDescriptorSubtype *\/ */
  /* AUDIO_IN_STREAMING_CTRL,              /\* bUnitID *\/ */
  /* 0x01,                                 /\* bSourceID *\/ */
  /* 0x01,                                 /\* bControlSize *\/ */
  /* AUDIO_CONTROL_VOLUME,                 /\* bmaControls(0) *\/ */
  /* /\* AUDIO_CONTROL_MUTE|AUDIO_CONTROL_VOLUME,                 /\\* bmaControls(0) *\\/ *\/ */
  /* 0,                 /\* bmaControls(1) *\/ */
  /* 0,                 /\* bmaControls(2) *\/ */
  /* 0,                 /\* bmaControls(3) *\/ */
  /* 0,                 /\* bmaControls(4) *\/ */
  /* 0x00,                                 /\* iFeature *\/ */
 
  /* USB Microphone Standard AS Interface Descriptor (Alt. Set. 0) (CODE == 3)*/ //zero-bandwidth interface
  0x09,                         // Size of the descriptor, in bytes (bLength)
  USB_DESC_TYPE_INTERFACE,    // INTERFACE descriptor type (bDescriptorType) 0x04
  0x01, // Index of this interface. (bInterfaceNumber) ?????????? (3<) (1<<) (1<M)
  0x00,                         // Index of this alternate setting. (bAlternateSetting)
  0x00,                         // 0 endpoints.   (bNumEndpoints)
  USB_DEVICE_CLASS_AUDIO,       // AUDIO (bInterfaceClass)
  AUDIO_SUBCLASS_AUDIOSTREAMING, // AUDIO_STREAMING (bInterfaceSubclass)
  0x00,                         // Unused. (bInterfaceProtocol)
  0x00,                         // Unused. (iInterface)
 
  /* USB Microphone Standard AS Interface Descriptor (Alt. Set. 1) (CODE == 4)*/
  0x09,                         // Size of the descriptor, in bytes (bLength)
  USB_DESC_TYPE_INTERFACE,     // INTERFACE descriptor type (bDescriptorType)
  0x01, // Index of this interface. (bInterfaceNumber)
  0x01,                         // Index of this alternate setting. (bAlternateSetting)
  0x01,                         // 1 endpoint (bNumEndpoints)
  USB_DEVICE_CLASS_AUDIO,       // AUDIO (bInterfaceClass)
  AUDIO_SUBCLASS_AUDIOSTREAMING,   // AUDIO_STREAMING (bInterfaceSubclass)
  0x00,                         // Unused. (bInterfaceProtocol)
  0x00,                         // Unused. (iInterface)
 
  /*  USB Microphone Class-specific AS General Interface Descriptor (CODE == 5)*/
  0x07,                         // Size of the descriptor, in bytes (bLength)
  AUDIO_INTERFACE_DESCRIPTOR_TYPE, // CS_INTERFACE Descriptor Type (bDescriptorType) 0x24
  AUDIO_STREAMING_GENERAL,         // GENERAL subtype (bDescriptorSubtype) 0x01
  0x02,             // Unit ID of the Output Terminal.(bTerminalLink)
  0x00,                         // Interface delay. (bDelay)
  0x01,0x00,                    // PCM Format (wFormatTag) The document 'USB Audio Data formats' contains tag definitions
 
  /*  USB Microphone Type I Format Type Descriptor (CODE == 6)*/
  0x0B,                        // Size of the descriptor, in bytes (bLength)
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,// CS_INTERFACE Descriptor Type (bDescriptorType) 0x24
  AUDIO_STREAMING_FORMAT_TYPE,   // FORMAT_TYPE subtype. (bDescriptorSubtype) 0x02
  0x01,                        // FORMAT_TYPE_I. (bFormatType)
  USB_AUDIO_CHANNELS,              // Audio channels.(bNrChannels)
  AUDIO_BYTES_PER_SAMPLE,       // Two bytes per audio subframe.(bSubFrameSize)
  AUDIO_BITS_PER_SAMPLE,       // 16 bits per sample.(bBitResolution)
  0x01,                        // One frequency supported. (bSamFreqType)
  AUDIO_SAMPLE_FREQ(USBD_AUDIO_FREQ),         /* Audio sampling frequency coded on 3 bytes */
 
  /*  USB Microphone Standard Endpoint Descriptor (CODE == 8)*/ //Standard AS Isochronous Audio Data Endpoint Descriptor
  0x09,                       // Size of the descriptor, in bytes (bLength)
  0x05,                       // ENDPOINT descriptor (bDescriptorType)
  AUDIO_IN_EP,                    // IN Endpoint 1. (bEndpointAddress)
  USBD_EP_TYPE_ISOC,                    /* bmAttributes (1) */
  AUDIO_PACKET_SZE(USBD_AUDIO_FREQ),    /* wMaxPacketSize in Bytes (Freq(Samples)*2(Stereo)*2(HalfWord)) */
  0x01,                       // Polling interval 1kHz. (bInterval)
  0x00,                       // Unused. (bRefresh)
  0x00,                       // Unused. (bSynchAddress)
 
  /* USB Microphone Class-specific Isoc. Audio Data Endpoint Descriptor (CODE == 7) OK - */
  0x07,                       // Size of the descriptor, in bytes (bLength)
  AUDIO_ENDPOINT_DESCRIPTOR_TYPE,    // CS_ENDPOINT Descriptor Type (bDescriptorType) 0x25
  AUDIO_ENDPOINT_GENERAL,            // GENERAL subtype. (bDescriptorSubtype) 0x01
  0x00,                              // No sampling frequency control, no pitch control, no packet padding.(bmAttributes)
  0x00,                              // Unused. (bLockDelayUnits)
  0x00,0x00,                         // Unused. (wLockDelay
 
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
  
  /* /\* Open EP OUT *\/ */
  /* USBD_LL_OpenEP(pdev, */
  /*                AUDIO_OUT_EP, */
  /*                USBD_EP_TYPE_ISOC, */
  /*                AUDIO_OUT_PACKET); */

    USBD_StatusTypeDef rv;

  /* rv = USBD_LL_OpenEP(pdev, */
  /* 		      AUDIO_OUT_EP, */
  /* 		      USBD_EP_TYPE_ISOC, */
  /* 		      AUDIO_OUT_PACKET); */
  /* if(rv != USBD_OK )  */
  /*   USBD_ErrLog("Open of OUT streaming endpoint failed. error %d\n", rv); */

  /* Open IN (i.e. microphone) Endpoint */
    rv = USBD_LL_OpenEP(pdev,
			AUDIO_IN_EP,
			USBD_EP_TYPE_ISOC,
			AUDIO_IN_PACKET_SIZE);
    if(rv != USBD_OK ) 
      USBD_ErrLog("Open of IN streaming endpoint failed. error %d\n", rv);

  /* /\* Allocate Audio structure *\/ */
  /* pdev->pClassData = USBD_malloc(sizeof (USBD_AUDIO_HandleTypeDef)); */
    pdev->pClassData = &usbd_audio_handle;
  
  if(pdev->pClassData == NULL)
  {
    return USBD_FAIL; 
  }
  else
  {
    haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;
    haudio->alt_setting = 0;
    haudio->offset = AUDIO_OFFSET_UNKNOWN;
    haudio->wr_ptr = 0; 
    haudio->rd_ptr = 0;  
    haudio->rd_enable = 0;
    
    /* Initialize the Audio output Hardware layer */
    if (((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->Init(USBD_AUDIO_FREQ, AUDIO_DEFAULT_VOLUME, 0) != USBD_OK)
    {
      return USBD_FAIL;
    }
    
    /* /\* Prepare Out endpoint to receive 1st packet *\/  */
    /* USBD_LL_PrepareReceive(pdev, */
    /*                        AUDIO_OUT_EP, */
    /*                        haudio->buffer,                         */
    /*                        AUDIO_OUT_PACKET);       */

    /* I'm not sure why this needs to be here, but if it is not, USBD_AUDIO_DataIn
     * doesn't get called */
    /* ringbuffer.read(mic_data, AUDIO_IN_PACKET_SIZE, true); */
    /* ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->AudioCmd(haudio->buffer, */
    /* 							 AUDIO_IN_PACKET_SIZE/2, */
    /* 							 AUDIO_CMD_START); */
    usbd_audio_start_callback(pdev, haudio);
  }
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

  /* /\* Close EP OUT *\/ */
  /* USBD_LL_CloseEP(pdev, */
  /*             AUDIO_OUT_EP); */

  /* Close EP IN */
  USBD_LL_CloseEP(pdev,
		  AUDIO_IN_EP);

  /* DeInit  physical Interface components */
  if(pdev->pClassData != NULL)
  {
   ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->DeInit(0);
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
        len = MIN(USB_AUDIO_DESC_SIZ , req->wLength);
        
        
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
  /* audio_tx_lock = 0; */
  /* USBD_DbgLog("DataIn\n"); */
  
  if(epnum != (AUDIO_IN_EP & ~0x80))
    return USBD_OK;

  USBD_AUDIO_HandleTypeDef *haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;

  /* ringbuffer.read(mic_data, AUDIO_IN_PACKET_SIZE, true); */
  /* ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->AudioCmd(&haudio->buffer[0], */
  /* 						       AUDIO_IN_PACKET_SIZE/2, */
  /* 						       AUDIO_CMD_PLAY); */
  usbd_audio_data_in_callback(pdev, haudio);
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
    if (haudio->control.unit == AUDIO_OUT_STREAMING_CTRL)
    {
     ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->MuteCtl(haudio->control.data[0]);     
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
    USBD_DbgLog("GET_CUR %d\n", haudio->control.unit);
  }else{
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
  if (haudio->control.cmd == AUDIO_REQ_SET_CUR)
  {/* In this driver, to simplify code, only SET_CUR request is managed */
    USBD_DbgLog("SET_CUR %d\n", haudio->control.unit);
    if (haudio->control.unit == AUDIO_OUT_STREAMING_CTRL)
    {
     ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->MuteCtl(haudio->control.data[0]);
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
    ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->AudioCmd(&haudio->buffer[0],
                                                         AUDIO_TOTAL_BUF_SIZE/2 - shift,
                                                         AUDIO_CMD_PLAY); 
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
  
  if (epnum == AUDIO_OUT_EP)
  {
    /* Increment the Buffer pointer or roll it back when all buffers are full */
    
    haudio->wr_ptr += AUDIO_OUT_PACKET;
    
    if (haudio->wr_ptr == AUDIO_TOTAL_BUF_SIZE)
    {/* All buffers are full: roll back */
      haudio->wr_ptr = 0;
      
      if(haudio->offset == AUDIO_OFFSET_UNKNOWN)
      {
        ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->AudioCmd(&haudio->buffer[0],
                                                             AUDIO_TOTAL_BUF_SIZE/2,
                                                             AUDIO_CMD_START);
          haudio->offset = AUDIO_OFFSET_NONE;
      }
    }
    
    if(haudio->rd_enable == 0)
    {
      if (haudio->wr_ptr == (AUDIO_TOTAL_BUF_SIZE / 2))
      {
        haudio->rd_enable = 1; 
      }
    }
    
    /* Prepare Out endpoint to receive next audio packet */
    USBD_LL_PrepareReceive(pdev,
                           AUDIO_OUT_EP,
                           &haudio->buffer[haudio->wr_ptr], 
                           AUDIO_OUT_PACKET);  
      
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
/**
  * @}
  */ 

void usbd_audio_write(USBD_HandleTypeDef* pdev, uint8_t* buf, uint32_t len) {
  if(pdev->dev_state == USBD_STATE_CONFIGURED){
    USBD_LL_Transmit(pdev, AUDIO_IN_EP, buf, len);
  }
}

/**
  * @}
  */ 


/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
