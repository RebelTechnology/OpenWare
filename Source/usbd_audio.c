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

USBD_AUDIO_HandleTypeDef usbd_audio_handle;

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

#ifdef USE_USBD_AUDIO
#define AUDIO_RX_EP                    0x01 // bEndpointAddress
#define AUDIO_TX_EP                    0x81
#define MIDI_RX_EP                     0x02
#define MIDI_TX_EP                     0x82
#ifdef USE_USBD_AUDIO_RX
#define USB_AUDIO_CONFIG_DESC_SIZ      258 // wTotalLength
#define AUDIO_NUM_INTERFACES           0x04
#define AUDIO_MIDI_IF                  0x03
#define AUDIO_TX_IF                    0x02
#define AUDIO_RX_IF                    0x01 // bInterfaceNumber
#else
#define USB_AUDIO_CONFIG_DESC_SIZ      174
#define AUDIO_NUM_INTERFACES           0x03
#define AUDIO_MIDI_IF                  0x02
#define AUDIO_TX_IF                    0x01
#define AUDIO_RX_IF                    0xff // dummy
#endif
#else
#define USB_AUDIO_CONFIG_DESC_SIZ      101
#define AUDIO_NUM_INTERFACES           0x02
#define AUDIO_MIDI_IF                  0x01
#define MIDI_RX_EP                     0x01
#define MIDI_TX_EP                     0x81
#define AUDIO_TX_IF                    0xff // dummy
#define AUDIO_RX_IF                    0xff // dummy
#endif

/* USB AUDIO device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_AUDIO_CfgDesc[USB_AUDIO_CONFIG_DESC_SIZ] __ALIGN_END =
{
 
  /* Configuration 1 */
  0x09,                                 /* bLength */
  0x02,                                 /* bDescriptorType */
  LOBYTE(USB_AUDIO_CONFIG_DESC_SIZ),    /* wTotalLength */
  HIBYTE(USB_AUDIO_CONFIG_DESC_SIZ),    /* wTotalLength */
  AUDIO_NUM_INTERFACES,                 /* bNumInterfaces */
  0x01,                                 /* bConfigurationValue */
  0x00,                                 /* iConfiguration */
  0x80,                                 /* bmAttributes: BUS Powered */
  USBD_MAX_POWER/2,                     /* bMaxPower in 2mA steps */
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

#ifdef USE_USBD_AUDIO
#ifdef USE_USBD_AUDIO_RX
  /* Class-Specific AC Interface Header Descriptor */
  0x0a,                                 // bLength
  0x24,                                 // bDescriptorType
  0x01,                                 // bDescriptorSubtype
  0x00,                                 // bcdADC
  0x01,                                 // bcdADC Audio Device compliant to the USB Audio specification version 1.00
  LOBYTE(62),                           // wTotalLength 10+12+10+9+12+9 = 62 (rx only is 41)
  HIBYTE(62),                           // wTotalLength
  // Includes the combined length of this descriptor header and all Unit and Terminal descriptors.
  0x02,                                 // bInCollection
  0x01,                                 // baInterfaceNr
  0x02,                                 // baInterfaceNr
  /* 10 bytes */

  /* USB IN Terminal for play session */
  /* Input Terminal Descriptor */
  0x0c,                                 // bLength
  0x24,                                 // bDescriptorType
  0x02,                                 // bDescriptorSubtype
  0x12,                                 // bTerminalID 
  0x01,                                 // wTerminalType USBD_AUDIO_TERMINAL_IO_USB_STREAMING   0x0101
  0x01,                                 // wTerminalType 
  0x00,                                 // bAssocTerminal
  USB_AUDIO_CHANNELS,                   // bNrChannels 
  0x00,                                 // wChannelConfig 0x03 sets stereo channels left and right
  0x00,                                 // wChannelConfig Mono sets no position bits 
  0x00,                                 // iChannelNames
  0x00,                                 // iTerminal Unused
  /* 12 byte */

  /* USB Play control feature */
  /* Feature Unit Descriptor*/
  0x0a,                                 // bLength
  0x24,                                 // bDescriptorType
  0x06,                                 // bDescriptorSubtype
  0x16,                                 // bUnitID
  0x12,                                 // bSourceID
  0x01,                                 // bControlSize
  /* @TODO add volume control on L/R channel */
  0x01|0x02, // USBD_AUDIO_CONTROL_FEATURE_UNIT_MUTE|USBD_AUDIO_CONTROL_FEATURE_UNIT_VOLUME,      /* bmaControls(0) */
  0,                                            /* bmaControls(1) */
  0,                                            /* bmaControls(2) */
  0x00,                                         /* iTerminal */
  /* 10 byte */
  
  /*USB Play : Speaker Terminal */
  /* Output Terminal Descriptor */
  0x09,                                 // bLength
  0x024,                                // bDescriptorType
  0x03,                                 // bDescriptorSubtype
  0x14,                                 // bTerminalID 
  0x01,                                 // wTerminalType  0x0301
  0x03,                                 // wTerminalType  0x0301
  0x00,                                 // bAssocTerminal
  0x16,                                 // bSourceID FU 06
  0x00,                                 // iTerminal
  /* 09 byte */

  // 12+10+9 = 31 bytes
#else
  /* Class-Specific AC Interface Header Descriptor */
  0x09,                                 // bLength
  0x24,                                 // bDescriptorType
  0x01,                                 // bDescriptorSubtype HEADER descriptor subtype 0x01
  0x00,                         // bcdADC
  0x01,                         // bcdADC Audio Device compliant to the USB Audio specification version 1.00
  LOBYTE(30),                   // wTotalLength 9+12+9 = 30
  HIBYTE(30),                   // wTotalLength Total number of bytes returned for the class-specific AudioControl interface descriptor
  // Includes the combined length of this descriptor header and all Unit and Terminal descriptors.
  0x01,                         // bInCollection The number of AudioStreaming interfaces in the Audio Interface Collection to which this AudioControl interface belongs bInCollection
  0x01,                         // baInterfaceNr AudioStreaming interface 1 belongs to this AudioControl interface. baInterfaceNr
  /* 9 bytes */
#endif

#ifdef USE_USBD_AUDIO_TX  
  /*USB Microphone Input Terminal Descriptor */
  0x0C,                         // Size of the descriptor, in bytes
  0x24, // bDescriptorType CS_INTERFACE Descriptor Type 0x24
  0x02,    // bDescriptorSubtype INPUT_TERMINAL descriptor subtype 0x02
  0x01,                         // bTerminalID ID of this Terminal.
  0x01,                         // wTerminalType
  0x02,                         // wTerminalType Terminal is Microphone (0x0201)
  0x00,                         // bAssocTerminal No association
  USB_AUDIO_CHANNELS,           // bNrChannels 
  0x00,                         // wChannelConfig
  0x00,                         // wChannelConfig Mono sets no position bits 
  0x00,                         // iChannelNames Unused
  0x00,                         // iTerminal Unused
  /* 12 bytes */

  /* USB Microphone Output Terminal Descriptor */
  0x09,                            // Size of the descriptor, in bytes (bLength)
  0x24,                            // bDescriptorType
  0x03,                            // bDescriptorSubtype
  0x02,                            // bTerminalID
  0x01, 0x01,                      // wTerminalType
  0x00,                            // bAssocTerminal
  0x01,                            // From Input Terminal.(bSourceID)
  0x00,                            // unused  (iTerminal)
  /* 9 bytes */

  /* /\* USB Audio Feature Unit Descriptor *\/ */
  /* 0x09,                                 /\* bLength *\/ */
  /* 0x24,                                 /\* bDescriptorType *\/ */
  /* 0x06,                                 /\* bDescriptorSubtype *\/ */
  /* AUDIO_IN_STREAMING_CTRL,              /\* bUnitID *\/ */
  /* 0x01,                                 /\* bSourceID *\/ */
  /* 0x01,                                 /\* bControlSize *\/ */
  /* AUDIO_CONTROL_MUTE,                   /\* bmaControls(0) *\/ */
  /* 0,                                    /\* bmaControls(1) *\/ */
  /* 0x00,                                 /\* iTerminal *\/ */
  /* /\* 09 byte *\/ */

  /* USB Microphone Audio Feature Unit Descriptor */
  /* 0x0c,                                 /\* bLength *\/ */
  /* 0x24,                                 /\* bDescriptorType *\/ */
  /* 0x06,                                 /\* bDescriptorSubtype *\/ */
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
  /* 12 bytes */

  // 12+9 = 21 bytes
#endif
  
#ifdef USE_USBD_AUDIO_RX
  /* USB play Standard AS Interface Descriptor - Audio Streaming Zero Bandwith */
  /* Standard AS Interface Descriptor */
  0x09,                                    /* bLength */
  USB_DESC_TYPE_INTERFACE,                 /* bDescriptorType */
  AUDIO_RX_IF,                             /* bInterfaceNumber */
  0x00,                                    /* bAlternateSetting */
  0x00,                                    /* bNumEndpoints */
  0x01,                                    /* bInterfaceClass */
  0x02,                                    /* bInterfaceSubClass */
  0x00,                                    /* bInterfaceProtocol */
  0x00,                                    /* iInterface */
  /* 09 byte */
  
  /* USB play Standard AS Interface Descriptors - Audio streaming operational */
  /* Standard AS Interface Descriptor */
  0x09,                                    /* bLength */
  USB_DESC_TYPE_INTERFACE,                 /* bDescriptorType 0x04 */
  AUDIO_RX_IF,                             /* bInterfaceNumber */
  0x01,                                    /* bAlternateSetting */
  0x01,                                    /* bNumEndpoints */
  0x01,                                    /* bInterfaceClass */
  0x02,                                    /* bInterfaceSubClass */
  0x00,                                    /* bInterfaceProtocol */
  0x00,                                    /* iInterface */
  /* 09 byte */
  
  /*Class-Specific AS Interface Descriptor */
  0x07,                                    /* bLength */
  0x24,                                    /* bDescriptorType */
  0x01,                                    /* bDescriptorSubtype */
  0x12,                                    /* bTerminalLink */
  0x01,                                    /* bDelay */
  0x01,                                    /* wFormatTag USBD_AUDIO_FORMAT_TYPE_PCM 0x0001*/
  0x00,
  /* 07 byte */

    /*  Audio Type I Format descriptor */
  0x0b,                                    /* bLength */
  0x24,                                    /* bDescriptorType */
  0x02,                                    /* bDescriptorSubtype */
  0x01,                                    /* bFormatType */ 
  USB_AUDIO_CHANNELS,                      /* bNrChannels */
  AUDIO_BYTES_PER_SAMPLE,                  /* bSubFrameSize */
  AUDIO_BITS_PER_SAMPLE,                   /* bBitResolution */
  0x01,                                    /* bSamFreqType only one frequency supported */ 
  AUDIO_SAMPLE_FREQ(USBD_AUDIO_FREQ),      /* Audio sampling frequency coded on 3 bytes */
  /* 11 byte */
  
  /* USB Play data ep  */
  /* Standard AS Isochronous Audio Data Endpoint Descriptor*/
  0x09,                                    /* bLength */
  USB_DESC_TYPE_ENDPOINT,                  /* bDescriptorType */
  AUDIO_RX_EP,                             /* bEndpointAddress */
  USBD_EP_TYPE_ISOC|USBD_EP_ATTR_ISOC_SYNC,                       /* bmAttributes */
  AUDIO_PACKET_SZE(USBD_AUDIO_FREQ),       /* wMaxPacketSize in Bytes (Freq(Samples)*2(Stereo)*2(HalfWord)) */
  0x01,                                    /* bInterval */
  0x00,                                    /* bRefresh */
  0x00,                                    /* bSynchAddress */
  /* 09 byte */
  
  /* Class-Specific AS Isochronous Audio Data Endpoint Descriptor*/
  0x07,                                    /* bLength */
  0x25,                                    /* bDescriptorType */
  0x01,                                    /* bDescriptor */
  0x00,                                    /* bmAttributes */
  0x00,                                    /* bLockDelayUnits */
  0x00,                                    /* wLockDelay */
  0x00,
  /* 07 byte */

  // 9+9+7+11+9+7 = 52 bytes
#endif /*USE_USBD_AUDIO_RX */

  /* USB Microphone Standard AS Interface Descriptor (Alt. Set. 0) (CODE == 3)*/ //zero-bandwidth interface
  0x09,                         // Size of the descriptor, in bytes (bLength)
  USB_DESC_TYPE_INTERFACE,      // INTERFACE descriptor type (bDescriptorType) 0x04
  AUDIO_TX_IF,                  // Index of this interface. (bInterfaceNumber)
  0x00,                         // Index of this alternate setting. (bAlternateSetting)
  0x00,                         // 0 endpoints.   (bNumEndpoints)
  0x01,                         // bInterfaceClass
  0x02,                         // bInterfaceSubclass
  0x00,                         // Unused. (bInterfaceProtocol)
  0x00,                         // Unused. (iInterface)
  /* 9 bytes */
 
  /* USB Microphone Standard AS Interface Descriptor (Alt. Set. 1) (CODE == 4)*/
  0x09,                         // Size of the descriptor, in bytes (bLength)
  USB_DESC_TYPE_INTERFACE,      // INTERFACE descriptor type (bDescriptorType) 0x04
  AUDIO_TX_IF,                  // Index of this interface. (bInterfaceNumber)
  0x01,                         // Index of this alternate setting. (bAlternateSetting)
  0x01,                         // 1 endpoint (bNumEndpoints)
  0x01,                         // bInterfaceClass
  0x02,                         // bInterfaceSubclass
  0x00,                         // Unused. (bInterfaceProtocol)
  0x00,                         // Unused. (iInterface)
  /* 9 bytes */
 
  /*  USB Microphone Class-specific AS General Interface Descriptor (CODE == 5)*/
  0x07,                         // Size of the descriptor, in bytes (bLength)
  0x24,                         // bDescriptorType
  0x01,                         // bDescriptorSubtype
  0x02,                         // Unit ID of the Output Terminal.(bTerminalLink)
  0x00,                         // Interface delay. (bDelay)
  0x01,
  0x00,                         // PCM Format (wFormatTag) See 'USB Audio Data formats'
  /* 7 bytes */
 
  /*  USB Microphone Type I Format Type Descriptor (CODE == 6)*/
  0x0B,                         // Size of the descriptor, in bytes (bLength)
  0x24,                         // bDescriptorType
  0x02,                         // bDescriptorSubtype
  0x01,                         // FORMAT_TYPE_I. (bFormatType)
  USB_AUDIO_CHANNELS ,          // Audio channels.(bNrChannels)
  AUDIO_BYTES_PER_SAMPLE,       // Two bytes per audio subframe.(bSubFrameSize)
  AUDIO_BITS_PER_SAMPLE,        // 16 bits per sample.(bBitResolution)
  0x01,                         // One frequency supported. (bSamFreqType)
  AUDIO_SAMPLE_FREQ(USBD_AUDIO_FREQ),         /* Audio sampling frequency coded on 3 bytes */
  /* 11 bytes */
 
  /*  USB Microphone Standard Endpoint Descriptor (CODE == 8)*/ //Standard AS Isochronous Audio Data Endpoint Descriptor
  0x09,                         // Size of the descriptor, in bytes (bLength)
  0x05,                         // ENDPOINT descriptor (bDescriptorType)
  AUDIO_TX_EP,                  // IN Endpoint 1. (bEndpointAddress)
  USBD_EP_TYPE_ISOC|USBD_EP_ATTR_ISOC_SYNC,            /* bmAttributes */ 
  AUDIO_PACKET_SZE(USBD_AUDIO_FREQ),    /* wMaxPacketSize in Bytes (Freq(Samples)*2(Stereo)*2(HalfWord)) */
  0x01,                         // Polling interval 1kHz. (bInterval)
  0x00,                         // Unused. (bRefresh)
  0x00,                         // Unused. (bSynchAddress)
  /* 09 bytes */
 
  /* USB Microphone Class-specific Isoc. Audio Data Endpoint Descriptor (CODE == 7) OK - */
  0x07,                              // Size of the descriptor, in bytes (bLength)
  0x25,                              // bDescriptorType
  0x01,                              // GENERAL subtype. (bDescriptorSubtype)
  0x00,                              // No sampling frequency control, no pitch control, no packet padding.(bmAttributes)
  0x00,                              // Unused. (bLockDelayUnits)
  0x00,0x00,                         // Unused. (wLockDelay
  /* 07 bytes */

#else
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
#endif
  
  /* Standard MS Interface Descriptor */
  /* MIDI Adapter Standard MS Interface Descriptor */
  0x09,                                 /* bLength */
  0x04,                                 /* bDescriptorType */
  AUDIO_MIDI_IF,                        /* bInterfaceNumber */
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
  0x41,                                 /* wTotalLength 7+6+6+9+9+9+5+9+5 = 65*/
  0x00,                                 /* wTotalLength */
  /* 07 bytes */

  /* MIDI Adapter MIDI IN Jack Descriptor (Embedded) */
  0x06,                                 /* bLength */
  0x24,                                 /* bDescriptorType */
  0x02,                                 /* bDescriptorSubtype */
  0x01,                                 /* bJackType */
  0x01,                                 /* bJackID */
  0x00,                                 /* iJack */
  /* 06 bytes */
  
  /* MIDI Adapter MIDI IN Jack Descriptor (External) */
  0x06,                                 /* bLength */
  0x24,                                 /* bDescriptorType */
  0x02,                                 /* bDescriptorSubtype */
  0x02,                                 /* bJackType */
  0x02,                                 /* bJackID */
  0x00,                                 /* iJack */
  /* 06 bytes */

  /* MIDI Adapter MIDI OUT Jack Descriptor (Embedded) */
  0x09,                                 /* bLength */
  0x24,                                 /* bDescriptorType */
  0x03,                                 /* bDescriptorSubtype */
  0x01,                                 /* bJackType */
  0x03,                                 /* bJackID */
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
  0x04,                                 /* bJackID */
  0x01,                                 /* bNrInputPins */
  0x01,                                 /* BaSourceID */
  0x01,                                 /* BaSourcePin */
  0x00,                                 /* iJack */
  /* 09 bytes */

  /* MIDI Adapter Standard Bulk OUT Endpoint Descriptor */
  0x09,                                 /* bLength */
  0x05,                                 /* bDescriptorType */
  MIDI_RX_EP,                          /* bEndpointAddress */
  0x02,                                 /* bmAttributes */
  0x40,                                 /* wMaxPacketSize */
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
  0x01,                                 /* BaAssocJackID */
  /* 05 bytes */

  /* MIDI Adapter Standard Bulk IN Endpoint Descriptor */
  0x09,                                 /* bLength */
  0x05,                                 /* bDescriptorType */
  MIDI_TX_EP,                           /* bEndpointAddress */
  0x02,                                 /* bmAttributes */
  0x40,                                 /* wMaxPacketSize */
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
  0x03                                  /* BaAssocJackID */
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

    /* Assign Audio structure */
    /* pdev->pClassData = USBD_malloc(sizeof (USBD_AUDIO_HandleTypeDef)); */
    pdev->pClassData = &usbd_audio_handle;  
    haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;
    haudio->tx_alt_setting = 0;
    haudio->rx_alt_setting = 0;
    haudio->midi_alt_setting = 0;
    haudio->midi_tx_lock = 0;
    haudio->audio_tx_active = 0;

#ifdef USE_USBD_AUDIO_TX
    /* Open IN (i.e. microphone) Endpoint */
    rv = USBD_LL_OpenEP(pdev,
			AUDIO_TX_EP,
			USBD_EP_TYPE_ISOC,
			AUDIO_TX_PACKET_SIZE);
    if(rv != USBD_OK)
      USBD_ErrLog("Open of IN streaming endpoint failed. error %d\n", rv);
    haudio->audio_tx_active = 1;
    usbd_audio_tx_start_callback(USBD_AUDIO_FREQ, USB_AUDIO_CHANNELS);
    usbd_audio_tx_callback(haudio->audio_tx_buffer, AUDIO_TX_PACKET_SIZE);
#endif
#ifdef USE_USBD_AUDIO_RX_FALSE
    /* Open OUT (i.e. speaker) Endpoint */
    rv = USBD_LL_OpenEP(pdev,
			AUDIO_RX_EP,
			USBD_EP_TYPE_ISOC,
			AUDIO_RX_PACKET_SIZE);
    if(rv != USBD_OK)
      USBD_ErrLog("Open of OUT streaming endpoint failed. error %d\n", rv);
    /* Prepare OUT endpoint to receive 1st packet */
    USBD_LL_PrepareReceive(pdev, AUDIO_RX_EP, haudio->audio_rx_buffer, AUDIO_RX_PACKET_SIZE);
    usbd_audio_rx_start_callback(USBD_AUDIO_FREQ, USB_AUDIO_CHANNELS);
#endif

#ifdef USE_USBD_MIDI
    /* Open the in EP */
    rv = USBD_LL_OpenEP(pdev,
			MIDI_TX_EP,
			USBD_EP_TYPE_BULK,
			MIDI_DATA_IN_PACKET_SIZE);
    if(rv != USBD_OK ) 
      USBD_ErrLog("Open of IN MIDI endpoint failed. error %d\n", rv);
    /* Open the out EP */
    rv = USBD_LL_OpenEP(pdev,
			MIDI_RX_EP,
			USBD_EP_TYPE_BULK,
			MIDI_DATA_OUT_PACKET_SIZE);			
    if(rv != USBD_OK )
      USBD_ErrLog("Open of OUT MIDI endpoint failed. error %d\n", rv);
    /* Prepare Out endpoint to receive next packet */
    USBD_LL_PrepareReceive(pdev,
			   MIDI_RX_EP,
			   haudio->midi_rx_buffer,
			   MIDI_DATA_OUT_PACKET_SIZE);
#endif

    return USBD_OK;
}

/**
  * @brief  USBD_AUDIO_DeInit
  *         DeInitialize the AUDIO layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_AUDIO_DeInit (USBD_HandleTypeDef *pdev, 
                                 uint8_t cfgidx)
{

#ifdef USE_USBD_MIDI
  USBD_LL_CloseEP(pdev, MIDI_TX_EP);
  USBD_LL_CloseEP(pdev, MIDI_RX_EP);
#endif

#ifdef USE_USBD_AUDIO_RX
  /* Close EP OUT */
  USBD_LL_CloseEP(pdev, AUDIO_RX_EP);		  
#endif

#ifdef USE_USBD_AUDIO_TX
  /* Close EP IN */
  USBD_LL_CloseEP(pdev, AUDIO_TX_EP);
#endif

  /* DeInit  physical Interface components */
  pdev->pClassData = NULL;

  return USBD_OK;
}

void usbd_audio_select_alt(USBD_HandleTypeDef* pdev, USBD_AUDIO_HandleTypeDef* haudio, uint8_t iface, uint8_t alt){
  USBD_StatusTypeDef rv;
  if(iface == AUDIO_TX_IF && haudio->tx_alt_setting != alt){
#ifdef USE_USBD_AUDIO_TX_FALSE
    if(alt == 0){
      // close previous
      haudio->audio_tx_active = 0;
      usbd_audio_tx_stop_callback();
      /* Close EP IN */
      USBD_LL_CloseEP(pdev, AUDIO_TX_EP);
    }
    if(alt == 1){
      /* Open IN (i.e. microphone) Endpoint */
      USBD_LL_FlushEP(pdev, AUDIO_TX_EP);
      rv = USBD_LL_OpenEP(pdev,
			  AUDIO_TX_EP,
			  USBD_EP_TYPE_ISOC,
			  AUDIO_TX_PACKET_SIZE);
      if(rv != USBD_OK)
	USBD_ErrLog("Open of IN streaming endpoint failed. error %d\n", rv);
      haudio->audio_tx_active = 1;
      usbd_audio_tx_start_callback(USBD_AUDIO_FREQ, USB_AUDIO_CHANNELS);
      usbd_audio_tx_callback(haudio->audio_tx_buffer, AUDIO_TX_PACKET_SIZE);
    }
#endif
    haudio->tx_alt_setting = alt;
  }else if(iface == AUDIO_RX_IF && haudio->rx_alt_setting != alt){
#ifdef USE_USBD_AUDIO_RX
    /* if(haudio->rx_alt_setting != 0){ */
    if(alt == 0){
      /* Close EP OUT */
      USBD_LL_CloseEP(pdev, AUDIO_RX_EP);
      usbd_audio_rx_stop_callback();
    }
    if(alt == 1){
      /* Open OUT (i.e. speaker) Endpoint */
      rv = USBD_LL_OpenEP(pdev,
			  AUDIO_RX_EP,
			  USBD_EP_TYPE_ISOC,
			  AUDIO_RX_PACKET_SIZE);
      if(rv != USBD_OK)
        USBD_ErrLog("Open of OUT streaming endpoint failed. error %d\n", rv);
      usbd_audio_rx_start_callback(USBD_AUDIO_FREQ, USB_AUDIO_CHANNELS);
      /* Prepare OUT endpoint to receive 1st packet */
      USBD_LL_PrepareReceive(pdev, AUDIO_RX_EP, haudio->audio_rx_buffer, AUDIO_RX_PACKET_SIZE);
    }      
#endif
    haudio->rx_alt_setting = alt;
  }
}

static uint8_t AUDIO_REQ(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{  
  uint8_t ret = USBD_OK;

      /* switch (req->bRequest) */
    /* { */
    /* case AUDIO_REQ_GET_CUR: */
    /*   AUDIO_REQ_GetCurrent(pdev, req); */
    /*   break; */
      
    /* case AUDIO_REQ_SET_CUR: */
    /*   AUDIO_REQ_SetCurrent(pdev, req);    */
    /*   break; */
      
    /* default: */
    /*   USBD_CtlError (pdev, req); */
    /*   ret = USBD_FAIL;  */
    /* } */

  static int16_t tmpdata;
  uint8_t control_selector = HIBYTE(req->wValue);
  switch(control_selector){
  case 0x01: { // USBD_AUDIO_CONTROL_FEATURE_UNIT_MUTE: {
    /* Send the current mute state */
    tmpdata = 0;
    USBD_CtlSendData (pdev, (uint8_t*)&tmpdata, 1);
    break;
  }
  case 0x02: { // USBD_AUDIO_CONTROL_FEATURE_UNIT_VOLUME: {
    switch(req->bRequest) {
    case AUDIO_REQ_GET_CUR:
      tmpdata = 0;
    case AUDIO_REQ_GET_MIN:
      tmpdata = -6400;
      /* tmpdata = (uint16_t*) &(feature_control->MinVolume); */
      break;
    case AUDIO_REQ_GET_MAX:
      tmpdata = 1536;
      /* tmpdata = (uint16_t*) &(feature_control->MaxVolume); */
      break;                         
    case AUDIO_REQ_GET_RES:
      tmpdata = 128;
      /* tmpdata = (uint16_t*) &(feature_control->ResVolume); */
      break;
    default :
      /* control not supported */
      ret = USBD_FAIL;      
      USBD_CtlError (pdev, req);
    }
    /* Send the current mute state */
    USBD_CtlSendData (pdev, (uint8_t*)&tmpdata, 2);
    break;
  }
  default:
    ret = USBD_FAIL;      
    USBD_CtlError (pdev, req);
  }
  /* if((req->bmRequest & USB_REQ_RECIPIENT_MASK) == USB_REQ_RECIPIENT_INTERFACE){ */
  /* }else{ */
  /*   ret = USBD_FAIL;       */
  /*    USBD_CtlError (pdev, req); */
  /* } */
  return ret;
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
  
  switch (req->bmRequest & USB_REQ_TYPE_MASK){
  case USB_REQ_TYPE_CLASS :  
    ret = AUDIO_REQ(pdev, req);
    break;
    
  case USB_REQ_TYPE_STANDARD:
    switch (req->bRequest) {
    case USB_REQ_GET_DESCRIPTOR:
      if( (req->wValue >> 8) == AUDIO_DESCRIPTOR_TYPE){
        pbuf = USBD_AUDIO_CfgDesc + 18;
        len = MIN(USB_AUDIO_CONFIG_DESC_SIZ , req->wLength);
        USBD_CtlSendData (pdev, pbuf, len);
      }
      break;
      
    case USB_REQ_GET_INTERFACE :
      switch(req->wIndex){
      case 0:
	haudio->control.data[0] = 0;
      	USBD_CtlSendData (pdev, haudio->control.data, 1);
	break;
#ifdef USE_USBD_AUDIO_RX
      case AUDIO_RX_IF:
	USBD_CtlSendData (pdev, &(haudio->rx_alt_setting), 1);
	break;
#endif
#ifdef USE_USBD_AUDIO_TX
      case AUDIO_TX_IF:
	USBD_CtlSendData (pdev, &(haudio->tx_alt_setting), 1);
	break;
#endif
#ifdef USE_USBD_MIDI
      case AUDIO_MIDI_IF:
	USBD_CtlSendData (pdev, &(haudio->midi_alt_setting), 1);
	break;
#endif
      default:
        ret = USBD_FAIL;
	break;
      }
      break;

    case USB_REQ_SET_INTERFACE :
      /* USBD_DbgLog("iface %d alt %d\n", req->wIndex, req->wValue); */
      switch(req->wIndex){
      case 0:
	/* Audio Control interface, only alternate zero is accepted  */     
	if(req->wValue != 0)
	  ret = USBD_FAIL;
	break;
#ifdef USE_USBD_AUDIO
      case AUDIO_RX_IF:
      case AUDIO_TX_IF:
	if(req->wValue <= 1) // only alt 0 or 1
	  usbd_audio_select_alt(pdev, haudio, req->wIndex, req->wValue);
	else
	  ret = USBD_FAIL;
	break;
#endif
      case AUDIO_MIDI_IF:
	// do nothing
	break;
      default:
	ret = USBD_FAIL;
	break;
      }
      break;
    default:
      ret = USBD_FAIL;
      break;
    }
  }
  if(ret == USBD_FAIL)
    USBD_CtlError (pdev, req);
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
  USBD_AUDIO_HandleTypeDef *haudio = (USBD_AUDIO_HandleTypeDef*)pdev->pClassData;
#ifdef USE_USBD_AUDIO_TX
  if(epnum == (AUDIO_TX_EP & ~0x80)){
    usbd_audio_tx_callback(haudio->audio_tx_buffer, AUDIO_TX_PACKET_SIZE);
  }
#endif
#ifdef USE_USBD_MIDI
  if(epnum == (MIDI_TX_EP & ~0x80)){
    haudio->midi_tx_lock = 0;
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
#ifdef USE_USBD_AUDIO
  USBD_AUDIO_HandleTypeDef   *haudio;
  haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;
  if (haudio->control.cmd == AUDIO_REQ_SET_CUR){
    USBD_DbgLog("SET_CUR %d\n", haudio->control.unit);
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
    USBD_DbgLog("GET_CUR %d\n", haudio->control.unit);
  }else{
    USBD_DbgLog("Control CMD %d\n", haudio->control.cmd);
  }
#endif
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
#ifdef USE_USBD_AUDIO
  USBD_AUDIO_HandleTypeDef   *haudio;
  haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;
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
#endif
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
  /* SOF (Start of Frame) Every millisecond (12000 full-bandwidth bit times), the USB host transmits a special SOF (start of frame) token, containing an 11-bit incrementing frame number in place of a device address. This is used to synchronize isochronous and interrupt data transfers. */
#ifdef USE_USBD_AUDIO
  usbd_audio_sync_callback(0);
#endif
  return USBD_OK;
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
  
#ifdef USE_USBD_AUDIO_RX
  if (epnum == AUDIO_RX_EP){
    uint32_t len = USBD_LL_GetRxDataSize(pdev, epnum);
    len = usbd_audio_rx_callback(haudio->audio_rx_buffer, len);
    /* Prepare Out endpoint to receive next audio packet */
    USBD_LL_PrepareReceive(pdev,
                           AUDIO_RX_EP,
                           haudio->audio_rx_buffer, 
                           len);
  }
#endif /* USE_USBD_AUDIO_RX */

#ifdef USE_USBD_MIDI
  if(epnum == MIDI_RX_EP){
    /* Forward data to midi callback */
    uint32_t len = USBD_LL_GetRxDataSize(pdev, epnum);
    usbd_midi_rx(haudio->midi_rx_buffer, len);
    /* Prepare Out endpoint to receive next packet */
    USBD_LL_PrepareReceive(pdev,
			   MIDI_RX_EP,
			   haudio->midi_rx_buffer,
			   MIDI_DATA_OUT_PACKET_SIZE);
#endif
  }
  
  return USBD_OK;
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
                                        void *fops)
{
  if(fops != NULL)
  {
    pdev->pUserData= fops;
  }
  return 0;
}

void usbd_audio_write(uint8_t* buf, size_t len) {
#ifdef USE_USBD_AUDIO_TX
  extern USBD_HandleTypeDef USBD_HANDLE;
  USBD_AUDIO_HandleTypeDef *haudio = (USBD_AUDIO_HandleTypeDef*)USBD_HANDLE.pClassData;
  if(USBD_HANDLE.dev_state == USBD_STATE_CONFIGURED && haudio->audio_tx_active)
    USBD_LL_Transmit(&USBD_HANDLE, AUDIO_TX_EP, buf, len);
#endif
}

void usbd_midi_tx(uint8_t* buf, uint32_t len) {
#ifdef USE_USBD_MIDI
  extern USBD_HandleTypeDef USBD_HANDLE;
  USBD_AUDIO_HandleTypeDef *haudio = (USBD_AUDIO_HandleTypeDef*)USBD_HANDLE.pClassData;
  if(USBD_HANDLE.dev_state == USBD_STATE_CONFIGURED && !haudio->midi_tx_lock){
    /* the call is non-blocking, and the DataIn callback of your USBD class is called with the endpoint number (excluding 0x80 bit) when the entire buffer has been transmitted over the endpoint */
    haudio->midi_tx_lock = 1;
    USBD_LL_Transmit(&USBD_HANDLE, MIDI_TX_EP, buf, len);
  }
#endif /* USE_USBD_MIDI */
}

uint8_t usbd_midi_connected(void){
#ifdef USE_USBD_MIDI
  extern USBD_HandleTypeDef USBD_HANDLE;
  return USBD_HANDLE.dev_state == USBD_STATE_CONFIGURED;
#else
  return 0;
#endif /* USE_USBD_MIDI */
}

uint8_t usbd_midi_ready(void){
  extern USBD_HandleTypeDef USBD_HANDLE;
  USBD_AUDIO_HandleTypeDef *haudio = (USBD_AUDIO_HandleTypeDef*)USBD_HANDLE.pClassData;
  /* return USBD_HANDLE.dev_state == USBD_STATE_CONFIGURED; */
  /* return USBD_HANDLE.ep_in.status == USBD_OK; */
  /* USBD_HANDLE.ep_out.status == USBD_OK */
  return USBD_HANDLE.dev_state == USBD_STATE_CONFIGURED &&
    /* USBD_HANDLE.ep_in && USBD_HANDLE.ep_in->status == USBD_OK && */
    haudio->midi_tx_lock == 0;
  /* return haudio->midi_tx_lock == 0; */
}
