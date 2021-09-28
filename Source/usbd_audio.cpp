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
#include "usbd_conf.h"
#include "usbd_ctlreq.h"

#include "CircularBuffer.h"

// todo: not static/global
CircularBuffer<audio_t> rx_buffer;
CircularBuffer<audio_t> tx_buffer;

#define AUDIO_SAMPLE_FREQ(frq)           (uint8_t)(frq), (uint8_t)((frq >> 8)), (uint8_t)((frq >> 16))
#define AUDIO_FREQ_FROM_DATA(bytes)      ((((uint32_t)((bytes)[2]))<<16)|(((uint32_t)((bytes)[1]))<<8)|(((uint32_t)((bytes)[0]))))
#define AUDIO_FREQ_TO_DATA(frq , bytes)  do{	\
    (bytes)[0]= (uint8_t)(frq);			\
    (bytes)[1]= (uint8_t)(((frq) >> 8));	\
    (bytes)[2]= (uint8_t)(((frq) >> 16));	\
  }while(0);


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

static void AUDIO_REQ_GetCurrent(USBD_HandleTypeDef* pdev, USBD_SetupReqTypedef* req);
static void AUDIO_REQ_GetMax(USBD_HandleTypeDef* pdev, USBD_SetupReqTypedef* req);
static void AUDIO_REQ_GetMin(USBD_HandleTypeDef* pdev, USBD_SetupReqTypedef* req);
static void AUDIO_REQ_GetRes(USBD_HandleTypeDef* pdev, USBD_SetupReqTypedef* req);
static void AUDIO_REQ_SetCurrent(USBD_HandleTypeDef* pdev, USBD_SetupReqTypedef* req);
#ifdef USE_USBD_AUDIO_RX
static void AUDIO_OUT_StopAndReset(USBD_HandleTypeDef* pdev);
static void AUDIO_OUT_Restart(USBD_HandleTypeDef* pdev);
#endif
static uint8_t VOL_PERCENT(int16_t vol);

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


#ifdef USE_USBD_RX_FB

#define FB_REFRESH 4 /* Feedback refresh rate 2^(10-FB_REFRESH) */
#define FB_RATE (1<<(10-FB_REFRESH))

/**
  * @brief   get_usb_full_speed_rate
  *         Set feedback value from sample rate 
  */
static void get_usb_full_speed_rate(unsigned int rate, uint8_t* buf){
  // convert sample rate to 16.16 format (from Linux kernel)
  uint32_t freq = ((rate << 13) + 62) / 125;
  // store 10.14 format in little endian
  buf[0] = freq >> 2;
  buf[1] = freq >> 10;
  buf[2] = freq >> 18;
}

uint8_t fb_data[3];

#endif

#if defined USE_USBD_AUDIO_RX && defined USE_USBD_RX_FB && defined USE_USBD_AUDIO_TX && defined USE_USBD_MIDI
#define AUDIO_RX_IF                    0x01 // bInterfaceNumber
#define AUDIO_TX_IF                    0x02
#define AUDIO_MIDI_IF                  0x03
#define AUDIO_RX_EP                    0x01 // bEndpointAddress
#define AUDIO_FB_EP                    0x81
#define AUDIO_TX_EP                    0x82
#define MIDI_RX_EP                     0x02
#define MIDI_TX_EP                     0x83
#elif defined USE_USBD_AUDIO_RX && defined USE_USBD_RX_FB && defined USE_USBD_AUDIO_TX
#define AUDIO_RX_IF                    0x01
#define AUDIO_TX_IF                    0x02
#define AUDIO_RX_EP                    0x01
#define AUDIO_FB_EP                    0x81
#define AUDIO_TX_EP                    0x82
#elif defined USE_USBD_AUDIO_RX && defined USE_USBD_RX_FB && defined USE_USBD_MIDI
#define AUDIO_RX_IF                    0x01
#define AUDIO_MIDI_IF                  0x02
#define AUDIO_RX_EP                    0x01
#define AUDIO_FB_EP                    0x81
#define MIDI_RX_EP                     0x02
#define MIDI_TX_EP                     0x82
#elif defined USE_USBD_AUDIO_RX && defined USE_USBD_AUDIO_TX && defined USE_USBD_MIDI
#define AUDIO_RX_IF                    0x01 // bInterfaceNumber
#define AUDIO_TX_IF                    0x02
#define AUDIO_MIDI_IF                  0x03
#define AUDIO_RX_EP                    0x01 // bEndpointAddress
#define AUDIO_TX_EP                    0x81
#define MIDI_RX_EP                     0x02
#define MIDI_TX_EP                     0x82
#elif defined USE_USBD_AUDIO_RX && defined USE_USBD_AUDIO_TX
#define AUDIO_RX_IF                    0x01
#define AUDIO_TX_IF                    0x02
#define AUDIO_RX_EP                    0x01
#define AUDIO_TX_EP                    0x81
#elif defined USE_USBD_AUDIO_RX && defined USE_USBD_MIDI
#define AUDIO_RX_IF                    0x01
#define AUDIO_MIDI_IF                  0x02
#define AUDIO_RX_EP                    0x01
#define MIDI_RX_EP                     0x02
#define MIDI_TX_EP                     0x81
#elif defined USE_USBD_AUDIO_TX && defined USE_USBD_MIDI
#define AUDIO_TX_IF                    0x01
#define AUDIO_MIDI_IF                  0x02
#define AUDIO_TX_EP                    0x81
#define MIDI_RX_EP                     0x02
#define MIDI_TX_EP                     0x82
#else
#define AUDIO_RX_IF                    0x01
#define AUDIO_TX_IF                    0x01
#define AUDIO_MIDI_IF                  0x01
#define AUDIO_RX_EP                    0x01
#define AUDIO_FB_EP                    0x81
#define AUDIO_TX_EP                    0x81
#define MIDI_RX_EP                     0x01
#define MIDI_TX_EP                     0x81
#endif

#ifdef USE_USBD_AUDIO_RX
#define USBD_AUDIO_RX_AC_DESC_LEN      30
#ifdef USE_USBD_RX_FB
#define USBD_AUDIO_RX_AS_DESC_LEN      61
#else
#define USBD_AUDIO_RX_AS_DESC_LEN      52
#endif
#define USBD_AUDIO_RX_NUM_INTERFACES   1
#else
#define USBD_AUDIO_RX_AC_DESC_LEN      0
#define USBD_AUDIO_RX_AS_DESC_LEN      0
#define USBD_AUDIO_RX_NUM_INTERFACES   0
#endif

#ifdef USE_USBD_AUDIO_TX
#define USBD_AUDIO_TX_AC_DESC_LEN      21
#define USBD_AUDIO_TX_AS_DESC_LEN      52
#define USBD_AUDIO_TX_NUM_INTERFACES   1
#else
#define USBD_AUDIO_TX_AC_DESC_LEN      0
#define USBD_AUDIO_TX_AS_DESC_LEN      0
#define USBD_AUDIO_TX_NUM_INTERFACES   0
#endif

#ifdef USE_USBD_MIDI
#define USBD_MIDI_DESC_LEN             74
#define USBD_MIDI_NUM_INTERFACES       1
#else
#define USBD_MIDI_DESC_LEN             0
#define USBD_MIDI_NUM_INTERFACES       0
#endif

// not including Audio Control AC interface
#define AUDIO_NUM_INTERFACES           (USBD_AUDIO_RX_NUM_INTERFACES+USBD_AUDIO_TX_NUM_INTERFACES+USBD_MIDI_NUM_INTERFACES)

#define USBD_AC_HEADER_LEN             (8+AUDIO_NUM_INTERFACES)
#define USBD_AC_DESC_LEN               (USBD_AC_HEADER_LEN+USBD_AUDIO_RX_AC_DESC_LEN+USBD_AUDIO_TX_AC_DESC_LEN)
#define USBD_AUDIO_CONFIG_DESC_SIZ      (9+9+USBD_AC_DESC_LEN+USBD_AUDIO_RX_AS_DESC_LEN+USBD_AUDIO_TX_AS_DESC_LEN+USBD_MIDI_DESC_LEN)

/* USB AUDIO device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_AUDIO_CfgDesc[USBD_AUDIO_CONFIG_DESC_SIZ] __ALIGN_END =
{
 
  /* Configuration 1 */
  0x09,                                 /* bLength */
  0x02,                                 /* bDescriptorType */
  LOBYTE(USBD_AUDIO_CONFIG_DESC_SIZ),    /* wTotalLength */
  HIBYTE(USBD_AUDIO_CONFIG_DESC_SIZ),    /* wTotalLength */
  (AUDIO_NUM_INTERFACES+1),             /* bNumInterfaces (+1 for AC Interface) */
  0x01,                                 /* bConfigurationValue */
  0x00,                                 /* iConfiguration */
#if (USBD_SELF_POWERED == 1U)
  0xc0,                                 /* bmAttributes: Self Powered */
  0,                                    /* bMaxPower in 2mA steps */
#else
  0x80,                                 /* bmAttributes: BUS Powered */
  USBD_MAX_POWER,                       /* bMaxPower in 2mA steps */
#endif
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

  /* Class-Specific AC Interface Header Descriptor */
  USBD_AC_HEADER_LEN,                   // bLength
  0x24,                                 // bDescriptorType
  0x01,                                 // bDescriptorSubtype
  0x00,                                 // bcdADC
  0x01,                                 // bcdADC Audio Device compliant to the USB Audio specification version 1.00
  LOBYTE(USBD_AC_DESC_LEN),             // wTotalLength
  HIBYTE(USBD_AC_DESC_LEN),             // wTotalLength
  // Includes the combined length of this descriptor header and all Unit and Terminal descriptors.
  AUDIO_NUM_INTERFACES,                 // bInCollection
#ifdef USE_USBD_AUDIO_RX
  AUDIO_RX_IF,                          // baInterfaceNr
#endif
#ifdef USE_USBD_AUDIO_TX
  AUDIO_TX_IF,                          // baInterfaceNr
#endif
#ifdef USE_USBD_MIDI
  AUDIO_MIDI_IF,                        // baInterfaceNr
#endif
  /* 8+AUDIO_NUM_INTERFACES bytes */

#ifdef USE_USBD_AUDIO_RX
  /* USB Speaker Terminal Descriptor */
  0x0c,                                 // bLength
  0x24,                                 // bDescriptorType
  0x02,                                 // bDescriptorSubtype
  0x01,                                 // bTerminalID 
  0x01,                                 // wTerminalType USBD_AUDIO_TERMINAL_IO_USB_STREAMING   0x0101
  0x01,                                 // wTerminalType 
  0x00,                                 // bAssocTerminal
  USBD_AUDIO_RX_CHANNELS,                // bNrChannels
#if USBD_AUDIO_RX_CHANNELS == 1
  0x00,                                 // wChannelConfig 0x00 sets Mono, no position bits
#else
  0x03,                                 // wChannelConfig 0x03 sets stereo channels left and right
#endif
  0x00,                                 // wChannelConfig
  0x00,                                 // iChannelNames
  0x00,                                 // iTerminal Unused
  /* 12 byte */

  /* Feature Unit Descriptor*/
  0x09,                                 // bLength
  0x24,                                 // bDescriptorType
  0x06,                                 // bDescriptorSubtype
  0x02,                                 // bUnitID
  0x01,                                 // bSourceID
  0x01,                                 // bControlSize
  0x01|0x02,                            // bmaControls(0) AUDIO_CONTROL_MUTE|AUDIO_CONTROL_VOLUME
  0,                                    // bmaControls(1)
  0x00,                                 // iTerminal
  /* 09 byte */
  
  /* Output Terminal Descriptor */
  0x09,                                 // bLength
  0x024,                                // bDescriptorType
  0x03,                                 // bDescriptorSubtype
  0x03,                                 // bTerminalID 
  0x01,                                 // wTerminalType  0x0301
  0x03,                                 // wTerminalType  0x0301
  0x00,                                 // bAssocTerminal
  0x02,                                 // bSourceID
  0x00,                                 // iTerminal
  /* 09 byte */

  // 12+9+9 = 30 bytes
#endif

#ifdef USE_USBD_AUDIO_TX  
  /* USB Microphone Terminal Descriptor */
  0x0C,                         // Size of the descriptor, in bytes
  0x24,                         // bDescriptorType CS_INTERFACE Descriptor Type 0x24
  0x02,                         // bDescriptorSubtype INPUT_TERMINAL descriptor subtype 0x02
  0x01,                         // bTerminalID ID of this Terminal.
  0x01,                         // wTerminalType
  0x02,                         // wTerminalType Terminal is Microphone (0x0201)
  0x00,                         // bAssocTerminal No association
  USBD_AUDIO_TX_CHANNELS,        // bNrChannels 
  0x03,                         // wChannelConfig
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

  // 12+9 = 21 bytes
#endif
  
#ifdef USE_USBD_AUDIO_RX
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
  
  /* Standard AS Interface Descriptor */
  0x09,                                    /* bLength */
  USB_DESC_TYPE_INTERFACE,                 /* bDescriptorType 0x04 */
  AUDIO_RX_IF,                             /* bInterfaceNumber */
  0x01,                                    /* bAlternateSetting */
#ifdef USE_USBD_RX_FB
  0x02,                                    /* bNumEndpoints */
#else
  0x01,                                    /* bNumEndpoints */
#endif
  0x01,                                    /* bInterfaceClass */
  0x02,                                    /* bInterfaceSubClass */
  0x00,                                    /* bInterfaceProtocol */
  0x00,                                    /* iInterface */
  /* 09 byte */
  
  /*Class-Specific AS Interface Descriptor */
  0x07,                                    /* bLength */
  0x24,                                    /* bDescriptorType */
  0x01,                                    /* bDescriptorSubtype */
  0x01,                                    /* bTerminalLink */
  0x01,                                    /* bDelay */
  0x01,                                    /* wFormatTag AUDIO_FORMAT_PCM 0x0001*/
  0x00,
  /* 07 byte */

    /*  Audio Type I Format descriptor */
  0x0b,                                    /* bLength */
  0x24,                                    /* bDescriptorType */
  0x02,                                    /* bDescriptorSubtype */
  0x01,                                    /* bFormatType */ 
  USBD_AUDIO_RX_CHANNELS,                  /* bNrChannels */
  AUDIO_BYTES_PER_SAMPLE,                  /* bSubFrameSize */
  AUDIO_BITS_PER_SAMPLE,                   /* bBitResolution */
  0x01,                                    /* bSamFreqType only one frequency supported */ 
  AUDIO_SAMPLE_FREQ(USBD_AUDIO_RX_FREQ),   /* Audio sampling frequency coded on 3 bytes */
  /* 11 byte */
  
  /* USB Play data ep  */
#ifdef USE_USBD_RX_FB
  /* Standard AS Isochronous asynchronous Audio Data Endpoint Descriptor*/
  0x09,                                    /* bLength */
  USB_DESC_TYPE_ENDPOINT,                  /* bDescriptorType */
  AUDIO_RX_EP,                             /* bEndpointAddress 1 out endpoint*/
  USBD_EP_TYPE_ISOC|USBD_EP_ATTR_ISOC_ADAPT|USBD_EP_ATTR_ISOC_DATA, /* bmAttributes */
  LOBYTE(AUDIO_RX_MAX_PACKET_SIZE),        /* wMaxPacketSize in bytes */
  HIBYTE(AUDIO_RX_MAX_PACKET_SIZE),
  0x01,                                    /* bInterval */
  0x00,                                    /* bRefresh */
  AUDIO_FB_EP,                             /* bSynchAddress */
  /* 09 byte */
#else
  0x09,                                    /* bLength */
  USB_DESC_TYPE_ENDPOINT,                  /* bDescriptorType */
  AUDIO_RX_EP,                             /* bEndpointAddress 1 out endpoint*/
  USBD_EP_TYPE_ISOC|USBD_EP_ATTR_ISOC_ASYNC|USBD_EP_ATTR_ISOC_DATA, /* bmAttributes */
  LOBYTE(AUDIO_RX_MAX_PACKET_SIZE),		   /* wMaxPacketSize in bytes */
  HIBYTE(AUDIO_RX_MAX_PACKET_SIZE),
  0x01,                                    /* bInterval */
  0x00,                                    /* bRefresh */
  0,                                       /* bSynchAddress */
  /* 09 byte */
#endif
  
  /* Class-Specific AS Isochronous Audio Data Endpoint Descriptor*/
  0x07,                                    /* bLength */
  0x25,                                    /* bDescriptorType */
  0x01,                                    /* bDescriptor */
  0x00,                                    /* bmAttributes 0x01: Sampling Frequency control. See UAC Spec 1.0 p.62 */
  0x00,                                    /* bLockDelayUnits */
  0x00,                                    /* wLockDelay */
  0x00,
  /* 07 byte */

#ifdef USE_USBD_RX_FB
  /* Endpoint 2 - Standard Descriptor - See UAC Spec 1.0 p.63 4.6.2.1 Standard AS Isochronous Synch Endpoint Descriptor */
  0x09,                                    /* bLength */
  USB_DESC_TYPE_ENDPOINT,                  /* bDescriptorType */
  AUDIO_FB_EP,                             /* bEndpointAddress */
  USBD_EP_TYPE_ISOC|USBD_EP_ATTR_ISOC_NOSYNC|USBD_EP_ATTR_ISOC_FB,  /* bmAttributes */
  LOBYTE(AUDIO_RX_MAX_PACKET_SIZE),		   /* wMaxPacketSize */
  HIBYTE(AUDIO_RX_MAX_PACKET_SIZE),
  0x01,                                    /* bInterval 1ms */
  FB_REFRESH,                              /* bRefresh SOF_RATE */
  0x00,                                    /* bSynchAddress */
  /* 09 byte*/
#endif

  // 9+9+7+11+9+7+9 = 61 bytes
#endif /*USE_USBD_AUDIO_RX */

#ifdef USE_USBD_AUDIO_TX
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
  USBD_AUDIO_TX_CHANNELS,       // Audio channels.(bNrChannels)
  AUDIO_BYTES_PER_SAMPLE,       // Two bytes per audio subframe.(bSubFrameSize)
  AUDIO_BITS_PER_SAMPLE,        // 16 bits per sample.(bBitResolution)
  0x01,                         // One frequency supported. (bSamFreqType)
  AUDIO_SAMPLE_FREQ(USBD_AUDIO_TX_FREQ), // Audio sampling frequency coded on 3 bytes
  /* 11 bytes */
 
  /*  USB Microphone Standard Endpoint Descriptor (CODE == 8)*/ //Standard AS Isochronous Audio Data Endpoint Descriptor
  0x09,                         // Size of the descriptor, in bytes (bLength)
  0x05,                         // ENDPOINT descriptor (bDescriptorType)
  AUDIO_TX_EP,                  // IN Endpoint 1. (bEndpointAddress)
  // USBD_EP_TYPE_ISOC|USBD_EP_ATTR_ISOC_SYNC,   // bmAttributes
  USBD_EP_TYPE_ISOC|USBD_EP_ATTR_ISOC_ASYNC|USBD_EP_ATTR_ISOC_DATA,   // bmAttributes
  // USBD_EP_TYPE_ISOC,   // bmAttributes (device not recognised in Windows if sync or async)
  LOBYTE(AUDIO_TX_MAX_PACKET_SIZE),	// wMaxPacketSize in bytes
  HIBYTE(AUDIO_TX_MAX_PACKET_SIZE),
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

  // 9+9+7+11+9+7 = 52 bytes
#endif /*USE_USBD_AUDIO_TX */

#ifdef USE_USBD_MIDI
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
  MIDI_RX_EP,                           /* bEndpointAddress */
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

  // 5+9+5+9+9+9+6+6+7+9 = 74 bytes
#endif /* USE_USBD_MIDI */
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

static USBD_StatusTypeDef USBD_AUDIO_CloseEndpoint(USBD_HandleTypeDef *pdev,
						   USBD_AUDIO_HandleTypeDef* haudio, uint8_t ep){
  USBD_StatusTypeDef rv;
  rv = USBD_LL_CloseEP(pdev, ep);
  if(ep & 0x80)
    pdev->ep_out[ep & 0xFU].is_used = 0U;
  else
    pdev->ep_in[ep & 0xFU].is_used = 0U;
  return rv;
}

static USBD_StatusTypeDef USBD_AUDIO_OpenEndpoint(USBD_HandleTypeDef *pdev,
						  USBD_AUDIO_HandleTypeDef* haudio,
						  uint8_t ep,
						  uint8_t type,
						  uint16_t packet_size){
  USBD_StatusTypeDef rv;
  if(ep & 0x80){
    rv = USBD_LL_OpenEP(pdev, ep, type, packet_size);
    if(rv == USBD_OK){
      pdev->ep_in[ep & 0xFU].is_used = 1U;
      /* Flush endpoint */
      USBD_LL_FlushEP(pdev, ep);
    }
  }else{
    rv = USBD_LL_OpenEP(pdev, ep, type, packet_size);
    if(rv == USBD_OK)
      pdev->ep_out[ep & 0xFU].is_used = 1U;
  }
  if(rv != USBD_OK)
    USBD_ErrLog("Failed to open endpoint %d. error %d\n", ep, rv);
  return rv;
}

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
  USBD_DbgLog("Init 0x%x", cfgidx);

  USBD_AUDIO_HandleTypeDef   *haudio;
  /* Assign Audio structure */
  pdev->pClassData = &usbd_audio_handle;  
  haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;
  haudio->ac_alt_setting = 0;
  haudio->tx_alt_setting = 0;
  haudio->rx_alt_setting = 0;
  haudio->midi_alt_setting = 0;
  haudio->volume = 0;
#ifdef USE_USBD_AUDIO_RX
  rx_buffer.setData((audio_t*)haudio->audio_rx_buffer, AUDIO_RX_TOTAL_BUF_SIZE/sizeof(audio_t));
#endif
#ifdef USE_USBD_AUDIO_TX
  tx_buffer.setData((audio_t*)haudio->audio_tx_buffer, AUDIO_TX_TOTAL_BUF_SIZE/sizeof(audio_t));
#endif

#ifdef USE_USBD_MIDI
  USBD_AUDIO_OpenEndpoint(pdev, haudio, MIDI_TX_EP, USBD_EP_TYPE_BULK, MIDI_TX_PACKET_SIZE);
  USBD_AUDIO_OpenEndpoint(pdev, haudio, MIDI_RX_EP, USBD_EP_TYPE_BULK, MIDI_RX_PACKET_SIZE);
  /* Prepare Out endpoint to receive next packet */
  USBD_LL_PrepareReceive(pdev, MIDI_RX_EP, haudio->midi_rx_buffer, MIDI_RX_PACKET_SIZE);
  haudio->midi_tx_lock = 0;
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
  USBD_DbgLog("DeInit 0x%x", cfgidx);
  USBD_AUDIO_HandleTypeDef* haudio = (USBD_AUDIO_HandleTypeDef*)pdev->pClassData;
  (void)haudio;

#ifdef USE_USBD_AUDIO_RX
  USBD_AUDIO_CloseEndpoint(pdev, haudio, AUDIO_RX_EP);
#ifdef USE_USBD_RX_FB
  USBD_AUDIO_CloseEndpoint(pdev, haudio, AUDIO_FB_EP);
#endif
  haudio->audio_rx_active = 0;
  usbd_audio_rx_stop_callback();
#endif

#ifdef USE_USBD_AUDIO_TX
  USBD_AUDIO_CloseEndpoint(pdev, haudio, AUDIO_TX_EP);
  haudio->audio_tx_active = 0;
  usbd_audio_tx_stop_callback();
#endif

#ifdef USE_USBD_MIDI
  USBD_AUDIO_CloseEndpoint(pdev, haudio, MIDI_RX_EP);
  USBD_AUDIO_CloseEndpoint(pdev, haudio, MIDI_TX_EP);
  haudio->midi_tx_lock = 0;
#endif

  /* DeInit  physical Interface components */
  pdev->pClassData = NULL;

  return USBD_OK;
}

static uint8_t USBD_AUDIO_SetInterfaceAlternate(USBD_HandleTypeDef *pdev,
						uint8_t as_interface_num,
						uint8_t new_alt){
  USBD_DbgLog("SetInterfaceAlt 0x%x 0x%x", as_interface_num, new_alt);
  USBD_AUDIO_HandleTypeDef* haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;
  (void)haudio;
  uint8_t ret = USBD_OK;
  switch(as_interface_num){
#ifdef USE_USBD_AUDIO_RX
  case AUDIO_RX_IF:
    if(new_alt != haudio->rx_alt_setting){
      if(new_alt == 0){
    	// close old
	USBD_AUDIO_CloseEndpoint(pdev, haudio, AUDIO_RX_EP);
#ifdef USE_USBD_RX_FB
	USBD_AUDIO_CloseEndpoint(pdev, haudio, AUDIO_FB_EP);
#endif
	AUDIO_OUT_StopAndReset(pdev);
      }else{
    	// open new
    	USBD_AUDIO_OpenEndpoint(pdev, haudio, AUDIO_RX_EP, USBD_EP_TYPE_ISOC, AUDIO_RX_MAX_PACKET_SIZE);
#ifdef USE_USBD_RX_FB
	USBD_AUDIO_OpenEndpoint(pdev, haudio, AUDIO_FB_EP, USBD_EP_TYPE_ISOC, AUDIO_FB_PACKET_SIZE);
#endif
	haudio->fb_soffn = USB_SOF_NUMBER();
	/* haudio->bit_depth = 16U; // alt 1 */
	AUDIO_OUT_Restart(pdev);
      }
#ifdef USE_USBD_RX_FB
      USBD_LL_FlushEP(pdev, AUDIO_FB_EP);
#endif
      haudio->rx_alt_setting = new_alt;
    }    
    break;
#endif/* USE_USBD_AUDIO_RX */
#ifdef USE_USBD_AUDIO_TX
  case AUDIO_TX_IF:
    if(new_alt != haudio->tx_alt_setting){
      if(new_alt == 0){
  	// close old
  	USBD_AUDIO_CloseEndpoint(pdev, haudio, AUDIO_TX_EP);
	usbd_audio_tx_stop_callback();
	haudio->audio_tx_active = 0;
      }else{
  	// open new
  	USBD_AUDIO_OpenEndpoint(pdev, haudio, AUDIO_TX_EP, USBD_EP_TYPE_ISOC, AUDIO_TX_MAX_PACKET_SIZE);
	haudio->audio_tx_active = 1;
	haudio->tx_soffn = USB_SOF_NUMBER();
	usbd_audio_tx_start_callback(USBD_AUDIO_TX_FREQ, USBD_AUDIO_TX_CHANNELS, &tx_buffer);
	/* send first audio data */
	usbd_audio_write((uint8_t*)tx_buffer.getReadHead(), AUDIO_TX_PACKET_SIZE);	
	// usbd_audio_tx_callback(haudio->audio_tx_buffer, AUDIO_TX_PACKET_SIZE);
      }
      haudio->tx_alt_setting = new_alt;
    }
    break;
#endif /* USE_USBD_AUDIO_TX */
#ifdef USE_USBD_MIDI
  case AUDIO_MIDI_IF:
#endif
  case 0: // Control interface
    if(new_alt != 0)
      ret = USBD_FAIL;
    break;
  default:
    ret = USBD_FAIL;
    break;
  }
  return ret;
}

/**
 * @brief  USBD_AUDIO_Setup
 *         Handle the AUDIO specific requests
 * @param  pdev: instance
 * @param  req: usb requests
 * @retval status
 */
static uint8_t USBD_AUDIO_Setup (USBD_HandleTypeDef *pdev, 
				 USBD_SetupReqTypedef *req)
{
  USBD_AUDIO_HandleTypeDef   *haudio;
  uint16_t len;
  uint8_t *pbuf;
  uint8_t ret = USBD_OK;
  haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;
  uint16_t status_info = 0U;
  switch (req->bmRequest & USB_REQ_TYPE_MASK) {
    /* AUDIO Class Requests */
  case USB_REQ_TYPE_CLASS:
    switch (req->bRequest) {
    case AUDIO_REQ_GET_CUR:
      AUDIO_REQ_GetCurrent(pdev, req);
      break;
    case AUDIO_REQ_GET_MAX:
      AUDIO_REQ_GetMax(pdev, req);
      break;
    case AUDIO_REQ_GET_MIN:
      AUDIO_REQ_GetMin(pdev, req);
      break;
    case AUDIO_REQ_GET_RES:
      AUDIO_REQ_GetRes(pdev, req);
      break;
    case AUDIO_REQ_SET_CUR:
      AUDIO_REQ_SetCurrent(pdev, req);
      break;
    default:
      ret = USBD_FAIL;
      break;
    }
    break;
    /* Standard Requests */
  case USB_REQ_TYPE_STANDARD:
    switch (req->bRequest) {
    case USB_REQ_GET_STATUS:
      if (pdev->dev_state == USBD_STATE_CONFIGURED)
	USBD_CtlSendData(pdev, (uint8_t*)(void*)&status_info, 2U);
      else
	ret = USBD_FAIL;
      break;
    case USB_REQ_GET_DESCRIPTOR:
      if ((req->wValue >> 8) == AUDIO_DESCRIPTOR_TYPE) {
	pbuf = USBD_AUDIO_CfgDesc + 18;
	len = MIN(0x09, req->wLength);
	USBD_CtlSendData(pdev, pbuf, len);
      }
      break;
    case USB_REQ_GET_INTERFACE:
      switch(req->wIndex){
      case 0:
	USBD_CtlSendData(pdev, &(haudio->ac_alt_setting), 1);
	break;
#ifdef USE_USBD_AUDIO_RX
      case AUDIO_RX_IF:
	USBD_CtlSendData(pdev, &(haudio->rx_alt_setting), 1);
	break;
#endif
#ifdef USE_USBD_AUDIO_TX
      case AUDIO_TX_IF:
	USBD_CtlSendData(pdev, &(haudio->tx_alt_setting), 1);
	break;
#endif
#ifdef USE_USBD_MIDI
      case AUDIO_MIDI_IF:
	USBD_CtlSendData(pdev, &(haudio->midi_alt_setting), 1);
	break;
#endif
      default:
        ret = USBD_FAIL;
	break;
      }
      break;
    case USB_REQ_SET_INTERFACE:
      if (pdev->dev_state == USBD_STATE_CONFIGURED)
	ret = USBD_AUDIO_SetInterfaceAlternate(pdev, req->wIndex, req->wValue);
      else
	ret = USBD_FAIL;
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
  if(ret != USBD_OK)
    USBD_CtlError(pdev, req);
  /* USBD_DbgLog("Setup 0x%x 0x%x 0x%x 0x%x : 0x%x", req->bmRequest, req->bRequest, req->wIndex, req->wValue, ret); */
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
  (void)haudio;
  switch(epnum | 0x80){
#ifdef USE_USBD_RX_FB
  case AUDIO_FB_EP:
    haudio->fb_soffn = USB_SOF_NUMBER();
    USBD_LL_Transmit(pdev, AUDIO_FB_EP, (uint8_t*)fb_data, 3U);
    break;
#endif
#ifdef USE_USBD_AUDIO_TX
  case AUDIO_TX_EP:
    haudio->tx_soffn = USB_SOF_NUMBER();
    // todo: can we find out how much data was actually read?
    tx_buffer.moveReadHead(AUDIO_TX_PACKET_SIZE/sizeof(audio_t));
    // todo: write no more than available data
    usbd_audio_write((uint8_t*)tx_buffer.getReadHead(), AUDIO_TX_PACKET_SIZE);
    // usbd_audio_tx_callback(haudio->audio_tx_buffer, AUDIO_TX_PACKET_SIZE);
    break;
#endif
#ifdef USE_USBD_MIDI
  case MIDI_TX_EP:
    haudio->midi_tx_lock = 0;
    break;
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
static void AUDIO_REQ_GetCurrent(USBD_HandleTypeDef* pdev, USBD_SetupReqTypedef* req)
{
  USBD_AUDIO_HandleTypeDef* haudio;
  haudio = (USBD_AUDIO_HandleTypeDef*)pdev->pClassData;

  if ((req->bmRequest & 0x1f) == AUDIO_CONTROL_REQ) {
    switch (HIBYTE(req->wValue)) {
      case AUDIO_CONTROL_REQ_FU_MUTE: {
        /* Current mute state */
        uint8_t mute = 0;
        USBD_CtlSendData(pdev, &mute, 1);
      };
          break;
      case AUDIO_CONTROL_REQ_FU_VOL: {
        /* Current volume. See UAC Spec 1.0 p.77 */
        USBD_CtlSendData(pdev, (uint8_t*)&haudio->volume, 2);
      };
          break;
    }
  } else if ((req->bmRequest & 0x1f) == AUDIO_STREAMING_REQ) {
    if (HIBYTE(req->wValue) == AUDIO_STREAMING_REQ_FREQ_CTRL) {
      /* Current frequency */
      AUDIO_FREQ_TO_DATA(haudio->frequency, haudio->control.data)
      USBD_CtlSendData(pdev, haudio->control.data, 3);
    }
  }
}

/**
 * @brief  AUDIO_Req_GetMax
 *         Handles the GET_MAX Audio control request.
 * @param  pdev: instance
 * @param  req: setup class request
 * @retval status
 */
static void AUDIO_REQ_GetMax(USBD_HandleTypeDef* pdev, USBD_SetupReqTypedef* req)
{
  if ((req->bmRequest & 0x1f) == AUDIO_CONTROL_REQ) {
    switch (HIBYTE(req->wValue)) {
      case AUDIO_CONTROL_REQ_FU_VOL: {
        int16_t vol_max = USBD_AUDIO_VOL_MAX;
        USBD_CtlSendData(pdev, (uint8_t*)&vol_max, 2);
      };
	break;
    }
  /* } else if ((req->bmRequest & 0x1f) == AUDIO_STREAMING_REQ) { */
  /*   if (HIBYTE(req->wValue) == AUDIO_STREAMING_REQ_FREQ_CTRL) { */
  /*     // Max frequency */
  /*     AUDIO_FREQ_TO_DATA(data_ep->control_cbk.MaxFrequency , haudio->last_control.data) */
  /* 	USBD_CtlSendData(pdev, haudio->control.data, 3); */
  /*   } */
  }
}

/**
 * @brief  AUDIO_Req_GetMin
 *         Handles the GET_MIN Audio control request.
 * @param  pdev: instance
 * @param  req: setup class request
 * @retval status
 */
static void AUDIO_REQ_GetMin(USBD_HandleTypeDef* pdev, USBD_SetupReqTypedef* req)
{
  if ((req->bmRequest & 0x1f) == AUDIO_CONTROL_REQ) {
    switch (HIBYTE(req->wValue)) {
      case AUDIO_CONTROL_REQ_FU_VOL: {
        int16_t vol_min = USBD_AUDIO_VOL_MIN;
        USBD_CtlSendData(pdev, (uint8_t*)&vol_min, 2);
      };
          break;
    }
  /* } else if ((req->bmRequest & 0x1f) == AUDIO_STREAMING_REQ) { */
  /*   if (HIBYTE(req->wValue) == AUDIO_STREAMING_REQ_FREQ_CTRL) { */
  /*     // Min frequency */
  /*     AUDIO_FREQ_TO_DATA(data_ep->control_cbk.MinFrequency , haudio->last_control.data) */
  /* 	USBD_CtlSendData(pdev, haudio->control.data, 3); */
  /*   } */
  }
}

/**
 * @brief  AUDIO_Req_GetRes
 *         Handles the GET_RES Audio control request.
 * @param  pdev: instance
 * @param  req: setup class request
 * @retval status
 */
static void AUDIO_REQ_GetRes(USBD_HandleTypeDef* pdev, USBD_SetupReqTypedef* req)
{
  if ((req->bmRequest & 0x1f) == AUDIO_CONTROL_REQ) {
    switch (HIBYTE(req->wValue)) {
      case AUDIO_CONTROL_REQ_FU_VOL: {
        int16_t vol_res = USBD_AUDIO_VOL_STEP;
        USBD_CtlSendData(pdev, (uint8_t*)&vol_res, 2);
      };
          break;
    }
  }
}

/**
  * @brief  AUDIO_Req_SetCurrent
  *         Handles the SET_CUR Audio control request.
  * @param  pdev: instance
  * @param  req: setup class request
  * @retval status
  */
static void AUDIO_REQ_SetCurrent(USBD_HandleTypeDef* pdev, USBD_SetupReqTypedef* req)
{
  USBD_AUDIO_HandleTypeDef* haudio;
  haudio = (USBD_AUDIO_HandleTypeDef*)pdev->pClassData;

  if (req->wLength) {
    /* Prepare the reception of the buffer over EP0 */
    USBD_CtlPrepareRx(pdev,
                      haudio->control.data,
                      req->wLength);

    haudio->control.cmd = AUDIO_REQ_SET_CUR;          /* Set the request value */
    haudio->control.req_type = req->bmRequest & 0x1f; /* Set the request type. See UAC Spec 1.0 - 5.2.1 Request Layout */
    haudio->control.len = (uint8_t)req->wLength;      /* Set the request data length */
    haudio->control.unit = HIBYTE(req->wIndex);       /* Set the request target unit */
    haudio->control.cs = HIBYTE(req->wValue);         /* Set the request control selector (high byte) */
    haudio->control.cn = LOBYTE(req->wValue);         /* Set the request control number (low byte) */
  }
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
  (void)haudio;
#if 1
  if (haudio->control.cmd == AUDIO_REQ_SET_CUR) { /* In this driver, to simplify code, only SET_CUR request is managed */
    if (haudio->control.req_type == AUDIO_CONTROL_REQ) {
      USBD_DbgLog("CONTROL_REQ 0x%x 0x%x", haudio->control.cs, haudio->control.data[0]);
      switch (haudio->control.cs) {
	/* Mute Control */
      case AUDIO_CONTROL_REQ_FU_MUTE: {
	usbd_audio_mute_callback(haudio->control.data[0]);
	/* ((USBD_AUDIO_ItfTypeDef*)pdev->pUserData)->MuteCtl(haudio->control.data[0]); */
      };
	break;
	/* Volume Control */
      case AUDIO_CONTROL_REQ_FU_VOL: {
	int16_t vol = *(int16_t*)&haudio->control.data[0];
	haudio->volume = vol;
	usbd_audio_gain_callback(vol);
	/* ((USBD_AUDIO_ItfTypeDef*)pdev->pUserData)->VolumeCtl(VOL_PERCENT(vol)); */
      };
	break;
      }
    } else if (haudio->control.req_type == AUDIO_STREAMING_REQ) {
      USBD_DbgLog("STREAMING_REQ 0x%x 0x%x", haudio->control.cs, haudio->control.data[0]);
#ifdef USE_USBD_AUDIO_RX_FALSE // todo!
      if (haudio->control.cs == AUDIO_STREAMING_REQ_FREQ_CTRL) {
	/* Frequency Control */
	uint32_t new_freq = AUDIO_FREQ_FROM_DATA(haudio->control.data);
	if (haudio->frequency != new_freq) {
	  haudio->frequency = new_freq;
	  AUDIO_OUT_Restart(pdev);
	}
      }
#endif
    }
    haudio->control.req_type = 0U;
    haudio->control.cs = 0U;
    haudio->control.cn = 0U;
    haudio->control.cmd = 0U;
    haudio->control.len = 0U;
  }

#else
#ifdef USE_USBD_AUDIO
  if (haudio->control.cmd == AUDIO_REQ_SET_CUR){
    USBD_DbgLog("SET_CUR 0x%x", haudio->control.unit);
    if (haudio->control.unit == AUDIO_OUT_STREAMING_CTRL ||
	haudio->control.unit == AUDIO_IN_STREAMING_CTRL)
      {
	haudio->volume = (haudio->control.data[0] << 8) | haudio->control.data[1];
	usbd_audio_gain_callback(haudio->volume);
	haudio->control.cmd = 0;
	haudio->control.len = 0;
      }
  }else if (haudio->control.cmd == AUDIO_REQ_GET_CUR){
    USBD_DbgLog("GET_CUR 0x%x", haudio->control.unit);
  }else{
    USBD_DbgLog("Control CMD 0x%x 0x%x", haudio->control.cmd, haudio->control.unit);
  }
#endif
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
#ifdef USE_USBD_AUDIO_FALSE
  USBD_AUDIO_HandleTypeDef   *haudio;
  haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;
  if (haudio->control.cmd == AUDIO_REQ_SET_CUR)
  {/* In this driver, to simplify code, only SET_CUR request is managed */
    USBD_DbgLog("SET_CUR %d\n", haudio->control.unit);
    if (haudio->control.unit == AUDIO_OUT_STREAMING_CTRL ||
	haudio->control.unit == AUDIO_IN_STREAMING_CTRL)
    {
      haudio->volume = (haudio->control.data[0] << 8) | haudio->control.data[1];
      usbd_audio_gain_callback(haudio->volume);
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
static uint8_t  USBD_AUDIO_SOF (USBD_HandleTypeDef *pdev) {
  /* SOF (Start of Frame) Every millisecond the USB host transmits a special SOF (start of frame) token, containing an 11-bit incrementing frame number in place of a device address. This is used to synchronize isochronous and interrupt data transfers. */
  USBD_AUDIO_HandleTypeDef* haudio;
  haudio = (USBD_AUDIO_HandleTypeDef*)pdev->pClassData;  
#ifdef USE_USBD_RX_FB
  static uint32_t sof_count = 0;
  if(haudio->audio_rx_active && ++sof_count == FB_RATE){
    sof_count = 0;
    // number of samples since last request (or 0 if unknown)
    uint32_t samples = usbd_audio_get_rx_count();
    if(samples != 0){
      samples <<= (14 - FB_REFRESH); // convert to n.14 format
      AUDIO_FREQ_TO_DATA(samples, fb_data); // pack into 3 bytes
    }
  }
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
  
/* The application must read the Endpoint Control register for all isochronous IN */
/* endpoints to detect endpoints with incomplete IN data transfers. */
/* Endpoint transfer is incomplete if both the following conditions are met: */
/* - EONUM bit (in OTG_DOEPCTLx) = FNSOF[0] (in OTG_DSTS) */
/* - EPENA = 1 (in OTG_DOEPCTLx) */
  USBD_AUDIO_HandleTypeDef* haudio;
  haudio = (USBD_AUDIO_HandleTypeDef*)pdev->pClassData;  
  uint16_t current_sof;
  current_sof = USB_SOF_NUMBER();

#ifdef USE_USBD_RX_FB
  if(pdev->ep_in[AUDIO_FB_EP & 0xFU].is_used && 
     IS_ISO_IN_INCOMPLETE_EP(AUDIO_FB_EP & 0xFU, current_sof, haudio->fb_soffn)){
    USB_CLEAR_INCOMPLETE_IN_EP(AUDIO_FB_EP);
    USBD_LL_FlushEP(pdev, AUDIO_FB_EP);
    haudio->fb_soffn = USB_SOF_NUMBER();
    USBD_LL_Transmit(pdev, AUDIO_FB_EP, fb_data, 3U);
  }
#endif

#ifdef USE_USBD_AUDIO_TX
  if(pdev->ep_in[AUDIO_TX_EP & 0xFU].is_used && 
     IS_ISO_IN_INCOMPLETE_EP(AUDIO_TX_EP & 0xFU, current_sof, haudio->tx_soffn)){
    USB_CLEAR_INCOMPLETE_IN_EP(AUDIO_TX_EP);
    USBD_LL_FlushEP(pdev, AUDIO_TX_EP);
    haudio->tx_soffn = USB_SOF_NUMBER();
    // host has not collected data
    // let's not increment readhead
    // todo: write no more than available data
    usbd_audio_write((uint8_t*)tx_buffer.getReadHead(), AUDIO_TX_PACKET_SIZE);
// usbd_audio_tx_callback(haudio->audio_tx_buffer, AUDIO_TX_PACKET_SIZE);
  }
#endif
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
  USBD_AUDIO_HandleTypeDef *haudio;
  haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;
  (void)haudio;
#ifdef USE_USBD_AUDIO_RX
  if(epnum == AUDIO_RX_EP){
    uint32_t len;
    if(haudio->audio_rx_active == 0){
      haudio->audio_rx_active = 1;
      usbd_audio_rx_start_callback(USBD_AUDIO_RX_FREQ, USBD_AUDIO_RX_CHANNELS, &rx_buffer);
      len = AUDIO_RX_PACKET_SIZE;
#ifdef USE_USBD_RX_FB
      /* send first synchro data */
      get_usb_full_speed_rate(USBD_AUDIO_RX_FREQ, fb_data);
      USBD_LL_Transmit(pdev, AUDIO_FB_EP, fb_data, 3U);
#endif
    }else{
      len = USBD_LL_GetRxDataSize(pdev, epnum);
      rx_buffer.moveWriteHead(len/sizeof(audio_t));
    }
    // len = usbd_audio_rx_callback(haudio->audio_rx_buffer, len);
    // len = std::min(rx_buffer.getWriteCapacity(), AUDIO_RX_PACKET_SIZE);
    len = AUDIO_RX_PACKET_SIZE; // todo: ^
    /* Prepare Out endpoint to receive next audio packet */
    USBD_LL_PrepareReceive(pdev, AUDIO_RX_EP, (uint8_t*)rx_buffer.getWriteHead(), len);
  }
#endif /* USE_USBD_AUDIO_RX */
#ifdef USE_USBD_MIDI
  if(epnum == MIDI_RX_EP){
    /* Forward data to midi callback */
    uint32_t len = USBD_LL_GetRxDataSize(pdev, epnum);
    usbd_midi_rx(haudio->midi_rx_buffer, len);
    /* Prepare Out endpoint to receive next packet */
    USBD_LL_PrepareReceive(pdev, MIDI_RX_EP, haudio->midi_rx_buffer, MIDI_RX_PACKET_SIZE);
  }  
#endif
  return USBD_OK;
}


#ifdef USE_USBD_AUDIO_RX
/**
 * @brief  Stop playing and reset buffer pointers
 * @param  pdev: instance
 */
static void AUDIO_OUT_StopAndReset(USBD_HandleTypeDef* pdev)
{
  USBD_AUDIO_HandleTypeDef* haudio;
  haudio = (USBD_AUDIO_HandleTypeDef*)pdev->pClassData;
  USBD_DbgLog("AUDIO_OUT_StopAndReset %lu", haudio->frequency);

  haudio->audio_rx_active = 0;
#ifdef USE_USBD_RX_FB
  USBD_LL_FlushEP(pdev, AUDIO_FB_EP);
#endif
  USBD_LL_FlushEP(pdev, AUDIO_RX_EP);
  usbd_audio_rx_stop_callback();
  /* ((USBD_AUDIO_ItfTypeDef*)pdev->pUserData)->DeInit(0); */
}

/**
 * @brief  Restart playing with new parameters
 * @param  pdev: instance
 */
static void AUDIO_OUT_Restart(USBD_HandleTypeDef* pdev)
{
  USBD_AUDIO_HandleTypeDef* haudio;
  haudio = (USBD_AUDIO_HandleTypeDef*)pdev->pClassData;
  USBD_DbgLog("AUDIO_OUT_Restart %lu", haudio->frequency);

  /* AUDIO_OUT_StopAndReset(pdev); */
  haudio->audio_rx_active = 0;
#ifdef USE_USBD_RX_FB
  USBD_LL_FlushEP(pdev, AUDIO_FB_EP);
#endif
  USBD_LL_FlushEP(pdev, AUDIO_RX_EP);

  /* Prepare Out endpoint to receive first audio packet */
  // USBD_LL_PrepareReceive(pdev, AUDIO_RX_EP, haudio->audio_rx_buffer, AUDIO_RX_PACKET_SIZE);
  // size_t len = std::min(rx_buffer.getWriteCapacity(), AUDIO_RX_PACKET_SIZE);
  size_t len = AUDIO_RX_PACKET_SIZE; // todo: ^
  USBD_LL_PrepareReceive(pdev, AUDIO_RX_EP, (uint8_t*)rx_buffer.getWriteHead(), len);

  /* get_usb_full_speed_rate(haudio->frequency, fb_data); // reset to new frequency */

  /* usbd_audio_rx_start_callback(USBD_AUDIO_RX_FREQ, USBD_AUDIO_RX_CHANNELS); */
  /* ((USBD_AUDIO_ItfTypeDef*)pdev->pUserData)->Init(haudio->frequency, VOL_PERCENT(haudio->volume), 0); */
}
#endif /* USE_USBD_AUDIO_RX */


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
  pdev->pUserData = fops;
  return USBD_OK;
}

uint8_t  USBD_AUDIO_SetFiFos(PCD_HandleTypeDef *hpcd){
#if defined USE_USBD_AUDIO_RX && defined USE_USBD_AUDIO_TX && defined USE_USBD_MIDI
  HAL_PCDEx_SetRxFiFo(hpcd, 0x80);
  HAL_PCDEx_SetTxFiFo(hpcd, 0, 0x20);
  HAL_PCDEx_SetTxFiFo(hpcd, 1, 0x40);
  HAL_PCDEx_SetTxFiFo(hpcd, 2, 0x40);
  HAL_PCDEx_SetTxFiFo(hpcd, 3, 0x20);
#elif defined USE_USBD_AUDIO_RX && defined USE_USBD_AUDIO_TX
  HAL_PCDEx_SetRxFiFo(hpcd, 0x80);
  HAL_PCDEx_SetTxFiFo(hpcd, 0, 0x20);
  HAL_PCDEx_SetTxFiFo(hpcd, 1, 0x60);
  HAL_PCDEx_SetTxFiFo(hpcd, 2, 0x40);
#elif defined USE_USBD_AUDIO_RX && defined USE_USBD_MIDI
  HAL_PCDEx_SetRxFiFo(hpcd, 0x80);
  HAL_PCDEx_SetTxFiFo(hpcd, 0, 0x20);
  HAL_PCDEx_SetTxFiFo(hpcd, 1, 0x60);
  HAL_PCDEx_SetTxFiFo(hpcd, 2, 0x40);
#elif defined USE_USBD_AUDIO_TX && defined USE_USBD_MIDI
  HAL_PCDEx_SetRxFiFo(hpcd, 0x80);
  HAL_PCDEx_SetTxFiFo(hpcd, 0, 0x20);
  HAL_PCDEx_SetTxFiFo(hpcd, 1, 0x60);
  HAL_PCDEx_SetTxFiFo(hpcd, 2, 0x40);
#else
  HAL_PCDEx_SetRxFiFo(hpcd, 0xa0);
  HAL_PCDEx_SetTxFiFo(hpcd, 0, 0x40);
  HAL_PCDEx_SetTxFiFo(hpcd, 1, 0x60);
#endif
  /* total 0x140 words available for rx and tx fifos */  
  return USBD_OK;
}
  
/* Convert USB volume value to % */
uint8_t VOL_PERCENT(int16_t vol)
{
  return (uint8_t)((vol - (int16_t)USBD_AUDIO_VOL_MIN) / (((int16_t)USBD_AUDIO_VOL_MAX - (int16_t)USBD_AUDIO_VOL_MIN) / 100));
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
