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

#define AUDIO_SAMPLE_FREQ(frq)  (uint8_t)(frq), (uint8_t)((frq >> 8)), (uint8_t)((frq >> 16))


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

#if 1
static void AUDIO_REQ_GetCurrent(USBD_HandleTypeDef* pdev, USBD_SetupReqTypedef* req);
static void AUDIO_REQ_GetMax(USBD_HandleTypeDef* pdev, USBD_SetupReqTypedef* req);
static void AUDIO_REQ_GetMin(USBD_HandleTypeDef* pdev, USBD_SetupReqTypedef* req);
static void AUDIO_REQ_GetRes(USBD_HandleTypeDef* pdev, USBD_SetupReqTypedef* req);
static void AUDIO_REQ_SetCurrent(USBD_HandleTypeDef* pdev, USBD_SetupReqTypedef* req);
static void AUDIO_OUT_StopAndReset(USBD_HandleTypeDef* pdev);
static void AUDIO_OUT_Restart(USBD_HandleTypeDef* pdev);
static uint8_t VOL_PERCENT(int16_t vol);

#define USBD_AUDIO_DESC_TYPE_CS_DEVICE                               0x21
#define USBD_AUDIO_DESC_SIZ                                          0x09
#endif

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


#if 1

#if USBD_AUDIO_FREQ == 44100
#define AUDIO_FB_DEFAULT ((44 << 22) + (1 << 22) / 10)
#else
#define AUDIO_FB_DEFAULT ((USBD_AUDIO_FREQ/1000) << 22)
#endif

/* Feedback is limited to +/- 1kHz */
#define AUDIO_FB_DELTA (uint32_t)(1 << 22)

#define AUDIO_OUT_PACKET_24B           ((uint16_t)((USBD_AUDIO_FREQ / 1000U + 1) * 2U * 3U))
#define AUDIO_IN_PACKET                3U
volatile uint32_t tx_flag = 1;
volatile uint32_t is_playing = 0;
volatile uint32_t all_ready = 0;

volatile uint32_t fb_nom = AUDIO_FB_DEFAULT;
volatile uint32_t fb_value = AUDIO_FB_DEFAULT;
volatile uint32_t audio_buf_writable_size_last = AUDIO_RX_TOTAL_BUF_SIZE / 2U;
volatile int32_t fb_raw = AUDIO_FB_DEFAULT;
volatile uint8_t fb_data[3] = {
    (uint8_t)((AUDIO_FB_DEFAULT & 0x0000FF00) >> 8),
    (uint8_t)((AUDIO_FB_DEFAULT & 0x00FF0000) >> 16),
    (uint8_t)((AUDIO_FB_DEFAULT & 0xFF000000) >> 24)};

/* FNSOF is critical for frequency changing to work */
volatile uint32_t fnsof = 0;

#endif

#if defined USE_USBD_AUDIO_RX && defined USE_USBD_AUDIO_TX && defined USE_USBD_MIDI
#define AUDIO_RX_IF                    0x01 // bInterfaceNumber
#define AUDIO_TX_IF                    0x02
#define AUDIO_MIDI_IF                  0x03

#define AUDIO_RX_EP                    0x01 // bEndpointAddress
#define AUDIO_FB_EP                    0x81
#define AUDIO_TX_EP                    0x82
#define MIDI_RX_EP                     0x02
#define MIDI_TX_EP                     0x83

#elif defined USE_USBD_AUDIO_RX && defined USE_USBD_AUDIO_TX
#define AUDIO_RX_IF                    0x01
#define AUDIO_TX_IF                    0x02
#define AUDIO_RX_EP                    0x01 // bEndpointAddress
#define AUDIO_FB_EP                    0x81
#define AUDIO_TX_EP                    0x82

#define AUDIO_RX_EP                    0x01 // bEndpointAddress
#define AUDIO_FB_EP                    0x81
#define AUDIO_TX_EP                    0x82

#elif defined USE_USBD_AUDIO_RX && defined USE_USBD_MIDI
#define AUDIO_RX_IF                    0x01
#define AUDIO_MIDI_IF                  0x02

#define AUDIO_RX_EP                    0x01 // bEndpointAddress
#define AUDIO_FB_EP                    0x81
#define MIDI_RX_EP                     0x02
#define MIDI_TX_EP                     0x82

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

#define AUDIO_RX_EP                    0x01 // bEndpointAddress
#define AUDIO_FB_EP                    0x81
#define AUDIO_TX_EP                    0x81
#define MIDI_RX_EP                     0x01
#define MIDI_TX_EP                     0x81
#endif

#ifdef USE_USBD_AUDIO_RX
#define USBD_AUDIO_RX_AC_DESC_LEN      30
#define USBD_AUDIO_RX_AS_DESC_LEN      61
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
#define USB_AUDIO_CONFIG_DESC_SIZ      (9+9+USBD_AC_DESC_LEN+USBD_AUDIO_RX_AS_DESC_LEN+USBD_AUDIO_TX_AS_DESC_LEN+USBD_MIDI_DESC_LEN)

/* USB AUDIO device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_AUDIO_CfgDesc[USB_AUDIO_CONFIG_DESC_SIZ] __ALIGN_END =
{
 
  /* Configuration 1 */
  0x09,                                 /* bLength */
  0x02,                                 /* bDescriptorType */
  LOBYTE(USB_AUDIO_CONFIG_DESC_SIZ),    /* wTotalLength */
  HIBYTE(USB_AUDIO_CONFIG_DESC_SIZ),    /* wTotalLength */
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
  USB_AUDIO_CHANNELS,                   // bNrChannels
#if USB_AUDIO_CHANNELS == 1
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
  USB_AUDIO_CHANNELS,           // bNrChannels 
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
  0x02,                                    /* bNumEndpoints */
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
  USB_AUDIO_CHANNELS,                      /* bNrChannels */
  AUDIO_BYTES_PER_SAMPLE,                  /* bSubFrameSize */
  AUDIO_BITS_PER_SAMPLE,                   /* bBitResolution */
  0x01,                                    /* bSamFreqType only one frequency supported */ 
  AUDIO_SAMPLE_FREQ(USBD_AUDIO_FREQ),      /* Audio sampling frequency coded on 3 bytes */
  /* 11 byte */
  
  /* USB Play data ep  */
  /* Standard AS Isochronous asynchronous Audio Data Endpoint Descriptor*/
  0x09,                                    /* bLength */
  USB_DESC_TYPE_ENDPOINT,                  /* bDescriptorType */
  AUDIO_RX_EP,                             /* bEndpointAddress 1 out endpoint*/
  0x05,                                    /* bmAttributes */
  LOBYTE(AUDIO_RX_PACKET_SIZE),		   /* wMaxPacketSize in bytes */
  HIBYTE(AUDIO_RX_PACKET_SIZE),
  0x01,                                    /* bInterval */
  0x00,                                    /* bRefresh */
  AUDIO_FB_EP,                             /* bSynchAddress */
  /* 09 byte */
  
  /* Class-Specific AS Isochronous Audio Data Endpoint Descriptor*/
  0x07,                                    /* bLength */
  0x25,                                    /* bDescriptorType */
  0x01,                                    /* bDescriptor */
  0x01,                                    /* bmAttributes Sampling Frequency control supported. See UAC Spec 1.0 p.62 */
  0x00,                                    /* bLockDelayUnits */
  0x00,                                    /* wLockDelay */
  0x00,
  /* 07 byte */

    /* Endpoint 2 - Standard Descriptor - See UAC Spec 1.0 p.63 4.6.2.1 Standard AS Isochronous Synch Endpoint Descriptor */
    0x09,                              /* bLength */
    USB_DESC_TYPE_ENDPOINT,            /* bDescriptorType */
    AUDIO_FB_EP,                       /* bEndpointAddress */
    0x11,                              /* bmAttributes */
  LOBYTE(AUDIO_RX_PACKET_SIZE),		   /* wMaxPacketSize in bytes */
  HIBYTE(AUDIO_RX_PACKET_SIZE),
    0x01,                              /* bInterval 1ms */
    0x02,                          /* bRefresh SOF_RATE 4ms = 2^2 */
    0x00,                              /* bSynchAddress */
    /* 09 byte*/

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
  USBD_EP_TYPE_ISOC|USBD_EP_ATTR_ISOC_SYNC,/* bmAttributes */
  LOBYTE(AUDIO_TX_PACKET_SIZE),		   /* wMaxPacketSize in bytes */
  HIBYTE(AUDIO_TX_PACKET_SIZE),
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
  switch(ep){
#ifdef USE_USBD_AUDIO_RX
  case AUDIO_RX_EP:
    if(pdev->ep_out[AUDIO_RX_EP & 0xFU].is_used){
      usbd_audio_rx_stop_callback();
      rv = USBD_LL_CloseEP(pdev, AUDIO_RX_EP);
      pdev->ep_out[AUDIO_RX_EP & 0xFU].is_used = 0U;
    }
    break;
#endif /* USE_USBD_AUDIO_RX */
#ifdef USE_USBD_AUDIO_TX
  case AUDIO_TX_EP:
    if(pdev->ep_in[AUDIO_TX_EP & 0xFU].is_used){
      haudio->audio_tx_active = 0;
      usbd_audio_tx_stop_callback();
      rv = USBD_LL_CloseEP(pdev, AUDIO_TX_EP);
      pdev->ep_in[AUDIO_TX_EP & 0xFU].is_used = 0U;
    }
    break;
#endif /* USE_USBD_AUDIO_TX */
#ifdef USE_USBD_MIDI
  case MIDI_RX_EP:
    if(pdev->ep_out[MIDI_RX_EP & 0xFU].is_used){
      rv = USBD_LL_CloseEP(pdev, MIDI_RX_EP);
      pdev->ep_out[MIDI_RX_EP & 0xFU].is_used = 0U;
    }
    break;
  case MIDI_TX_EP:
    if(pdev->ep_in[MIDI_TX_EP & 0xFU].is_used){
      rv = USBD_LL_CloseEP(pdev, MIDI_TX_EP);
      pdev->ep_in[MIDI_TX_EP & 0xFU].is_used = 0U;
    }
    break;
#endif /* USE_USBD_MIDI */
  default:
    rv = USBD_FAIL;
  }
  return rv;
}

static USBD_StatusTypeDef USBD_AUDIO_OpenEndpoint(USBD_HandleTypeDef *pdev,
						  USBD_AUDIO_HandleTypeDef* haudio, uint8_t ep){
  USBD_StatusTypeDef rv = USBD_OK;
  switch(ep){
#ifdef USE_USBD_AUDIO_RX
  case AUDIO_RX_EP:
    if(!pdev->ep_out[AUDIO_RX_EP & 0xFU].is_used){
      /* Open OUT (i.e. speaker) Endpoint */
      rv = USBD_LL_OpenEP(pdev,
			  AUDIO_RX_EP,
			  USBD_EP_TYPE_ISOC,
			  AUDIO_RX_PACKET_SIZE);
      if(rv != USBD_OK)
	USBD_ErrLog("Open of OUT streaming endpoint failed. error %d\n", rv);
      pdev->ep_out[AUDIO_RX_EP & 0xFU].is_used = 1U;
      /* Prepare OUT endpoint to receive 1st packet */
      USBD_LL_PrepareReceive(pdev, AUDIO_RX_EP, haudio->audio_rx_buffer, AUDIO_RX_PACKET_SIZE);
      usbd_audio_rx_start_callback(USBD_AUDIO_FREQ, USB_AUDIO_CHANNELS);
    }
    break;
#endif /* USE_USBD_AUDIO_RX */
#ifdef USE_USBD_AUDIO_TX
  case AUDIO_TX_EP:
    if(!pdev->ep_in[AUDIO_TX_EP & 0xFU].is_used){
      /* Open IN (i.e. microphone) Endpoint */
      rv = USBD_LL_OpenEP(pdev,
			  AUDIO_TX_EP,
			  USBD_EP_TYPE_ISOC,
			  AUDIO_TX_PACKET_SIZE);
      if(rv != USBD_OK)
	USBD_ErrLog("Open of IN streaming endpoint failed. error %d\n", rv);
      pdev->ep_in[AUDIO_TX_EP & 0xFU].is_used = 1U;
      haudio->audio_tx_active = 1;
      usbd_audio_tx_start_callback(USBD_AUDIO_FREQ, USB_AUDIO_CHANNELS);
      USBD_LL_FlushEP(pdev, AUDIO_TX_EP);
      usbd_audio_tx_callback(haudio->audio_tx_buffer, AUDIO_TX_PACKET_SIZE);
    }
    break;
#endif /* USE_USBD_AUDIO_TX */
#ifdef USE_USBD_MIDI
  case MIDI_RX_EP:
    if(!pdev->ep_out[MIDI_RX_EP & 0xFU].is_used){
      /* Open the MIDI out EP */
      rv = USBD_LL_OpenEP(pdev,
			  MIDI_RX_EP,
			  USBD_EP_TYPE_BULK,
			  MIDI_DATA_OUT_PACKET_SIZE);			
      if(rv != USBD_OK )
	USBD_ErrLog("Open of OUT MIDI endpoint failed. error %d\n", rv);
      pdev->ep_out[MIDI_RX_EP & 0xFU].is_used = 1U;
      /* Prepare Out endpoint to receive next packet */
      USBD_LL_PrepareReceive(pdev,
			     MIDI_RX_EP,
			     haudio->midi_rx_buffer,
			     MIDI_DATA_OUT_PACKET_SIZE);
    }
    break;
  case MIDI_TX_EP:
    if(!pdev->ep_in[MIDI_TX_EP & 0xFU].is_used){
      /* Open the MIDI in EP */
      rv = USBD_LL_OpenEP(pdev,
			  MIDI_TX_EP,
			  USBD_EP_TYPE_BULK,
			  MIDI_DATA_IN_PACKET_SIZE);
      if(rv != USBD_OK)
	USBD_ErrLog("Open of IN MIDI endpoint failed. error %d\n", rv);
      pdev->ep_in[MIDI_TX_EP & 0xFU].is_used = 1U;
    }
    break;
#endif /* USE_USBD_MIDI */
  default:
    rv = USBD_FAIL;
  }
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

#if 1  
  /* Open EP OUT */
  USBD_LL_OpenEP(pdev, AUDIO_RX_EP, USBD_EP_TYPE_ISOC, AUDIO_OUT_PACKET_24B);
  pdev->ep_out[AUDIO_RX_EP & 0xFU].is_used = 1U;

  /* Open EP IN */
  USBD_LL_OpenEP(pdev, AUDIO_FB_EP, USBD_EP_TYPE_ISOC, AUDIO_IN_PACKET);
  pdev->ep_in[AUDIO_FB_EP & 0xFU].is_used = 1U;

  /* Flush feedback endpoint */
  USBD_LL_FlushEP(pdev, AUDIO_FB_EP);
#endif

  /** 
   * Set tx_flag 1 to block feedback transmission in SOF handler since 
   * device is not ready.
   */
  tx_flag = 1U;

  USBD_AUDIO_HandleTypeDef   *haudio;
  /* Assign Audio structure */
  pdev->pClassData = &usbd_audio_handle;  
  haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;
  haudio->ac_alt_setting = 0;
  haudio->tx_alt_setting = 0;
  haudio->rx_alt_setting = 0;
  haudio->midi_alt_setting = 0;
  haudio->midi_tx_lock = 0;
  haudio->audio_tx_active = 0;
  haudio->volume = 0;

#ifdef USE_USBD_AUDIO_TX_FALSE
  USBD_AUDIO_OpenEndpoint(pdev, haudio, AUDIO_TX_EP);
#endif

#ifdef USE_USBD_AUDIO_RX_FALSE
  USBD_AUDIO_OpenEndpoint(pdev, haudio, AUDIO_RX_EP);
#endif

#ifdef USE_USBD_MIDI
  USBD_AUDIO_OpenEndpoint(pdev, haudio, MIDI_TX_EP);
  USBD_AUDIO_OpenEndpoint(pdev, haudio, MIDI_RX_EP);
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

#if 1
    /* Flush all endpoints */
  USBD_LL_FlushEP(pdev, AUDIO_RX_EP);
  USBD_LL_FlushEP(pdev, AUDIO_FB_EP);

  /* Close EP OUT */
  USBD_LL_CloseEP(pdev, AUDIO_RX_EP);
  pdev->ep_out[AUDIO_RX_EP & 0xFU].is_used = 0U;

  /* Close EP IN */
  USBD_LL_CloseEP(pdev, AUDIO_FB_EP);
  pdev->ep_in[AUDIO_FB_EP & 0xFU].is_used = 0U;

  /* Clear feedback transmission flag */
  tx_flag = 0U;
#else
  
#ifdef USE_USBD_AUDIO_RX
  USBD_AUDIO_CloseEndpoint(pdev, haudio, AUDIO_RX_EP);
#endif

#endif

#ifdef USE_USBD_AUDIO_TX
  USBD_AUDIO_CloseEndpoint(pdev, haudio, AUDIO_TX_EP);
#endif

#ifdef USE_USBD_MIDI
  USBD_AUDIO_CloseEndpoint(pdev, haudio, MIDI_RX_EP);
  USBD_AUDIO_CloseEndpoint(pdev, haudio, MIDI_TX_EP);
#endif

  /* DeInit  physical Interface components */
  pdev->pClassData = NULL;

  return USBD_OK;
}

#if 0
static uint8_t USBD_AUDIO_SetInterfaceAlternate(USBD_HandleTypeDef *pdev,
						USBD_SetupReqTypedef *req){
  USBD_DbgLog("SetInterfaceAlt 0x%x 0x%x", req->wIndex, req->wValue);
  USBD_AUDIO_HandleTypeDef* haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;
  (void)haudio;
  uint8_t as_interface_num = req->wIndex;
  uint8_t new_alt = req->wValue;
  switch(as_interface_num){
#ifdef USE_USBD_AUDIO_RX
  case AUDIO_RX_IF:
    if(new_alt != haudio->rx_alt_setting){
      if(new_alt == 0){
    	// close old
	USBD_AUDIO_CloseEndpoint(pdev, haudio, AUDIO_RX_EP);
      }else{
    	// open new
    	USBD_AUDIO_OpenEndpoint(pdev, haudio, AUDIO_RX_EP);
      }
      haudio->rx_alt_setting = new_alt;
    }    
    return USBD_OK;
    break;
#endif/* USE_USBD_AUDIO_RX */
#ifdef USE_USBD_AUDIO_TX
  case AUDIO_TX_IF:
    if(new_alt != haudio->tx_alt_setting){
      if(new_alt == 0){
  	// close old
  	USBD_AUDIO_CloseEndpoint(pdev, haudio, AUDIO_TX_EP);
      }else{
  	// open new
  	USBD_AUDIO_OpenEndpoint(pdev, haudio, AUDIO_TX_EP);
      }
      haudio->tx_alt_setting = new_alt;
    }
    return USBD_OK;
    break;
#endif /* USE_USBD_AUDIO_TX */
#ifdef USE_USBD_MIDI
  case AUDIO_MIDI_IF:
    if(new_alt == 0)
      return USBD_OK;
    // deliberate fall-through
#endif
  case 0: // Control interface
  default:
    USBD_CtlError(pdev, req);
  }
 return USBD_FAIL;
}

static uint8_t AUDIO_REQ(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req) {  
  USBD_DbgLog("Req 0x%x 0x%x", req->wValue, req->bRequest);
  USBD_AUDIO_HandleTypeDef* haudio = (USBD_AUDIO_HandleTypeDef*)pdev->pClassData;
  static int16_t tmpdata;
  switch(HIBYTE(req->wValue)){
  case 0x01: // USBD_AUDIO_CONTROL_FEATURE_UNIT_MUTE: {
    /* Send the current mute state */
    tmpdata = 0;
    USBD_CtlSendData (pdev, (uint8_t*)&tmpdata, 1);
    break;
  case 0x02:  // USBD_AUDIO_CONTROL_FEATURE_UNIT_VOLUME: {
    switch(req->bRequest) {
    case AUDIO_REQ_GET_CUR:
      tmpdata = haudio->volume;
      break;
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
    case AUDIO_REQ_SET_CUR:
    case AUDIO_REQ_SET_RES:
      /* if (req->wLength) { */
      /* Prepare the reception of the buffer over EP0 */
      haudio->control.cmd = req->bRequest;     /* Set the request value */
      haudio->control.len = MIN(2, (uint8_t)req->wLength); /* Set the request data length */
      haudio->control.unit = HIBYTE(req->wIndex);  /* Set the request target unit */
      USBD_CtlPrepareRx(pdev, haudio->control.data, req->wLength);
      return USBD_OK;   
      /* } */
      break;
    default :
      /* control not supported */
      USBD_CtlError (pdev, req);
      return USBD_FAIL;
    }
    /* Send the response */
    USBD_CtlSendData (pdev, (uint8_t*)&tmpdata, 2);
    break;
  default:
    USBD_CtlError (pdev, req);
    return USBD_FAIL;
  }
  return USBD_OK;   
}
#endif

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
  USBD_DbgLog("Setup 0x%x 0x%x 0x%x 0x%x", req->bmRequest, req->bRequest, 
	      req->wIndex, req->wValue);
  USBD_AUDIO_HandleTypeDef   *haudio;
  uint16_t len;
  uint8_t *pbuf;
  uint8_t ret = USBD_OK;
  haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;
#if 1
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
      USBD_CtlError(pdev, req);
      ret = USBD_FAIL;
      break;
    }
    break;
    /* Standard Requests */
  case USB_REQ_TYPE_STANDARD:
    switch (req->bRequest) {
    case USB_REQ_GET_STATUS:
      if (pdev->dev_state == USBD_STATE_CONFIGURED) {
	USBD_CtlSendData(pdev, (uint8_t*)(void*)&status_info, 2U);
      } else {
	USBD_CtlError(pdev, req);
	ret = USBD_FAIL;
      }
      break;
    case USB_REQ_GET_DESCRIPTOR:
      if ((req->wValue >> 8) == AUDIO_DESCRIPTOR_TYPE) {
	pbuf = USBD_AUDIO_CfgDesc + 18;
	len = MIN(USBD_AUDIO_DESC_SIZ, req->wLength);
	USBD_CtlSendData(pdev, pbuf, len);
      }
      break;
    case USB_REQ_GET_INTERFACE:
      switch(req->wIndex){
      case 0:
	USBD_CtlSendData(pdev, &(haudio->ac_alt_setting), 1);
	return USBD_OK;
	break;
#ifdef USE_USBD_AUDIO_RX
      case AUDIO_RX_IF:
	USBD_CtlSendData(pdev, &(haudio->rx_alt_setting), 1);
	return USBD_OK;
	break;
#endif
#ifdef USE_USBD_AUDIO_TX
      case AUDIO_TX_IF:
	USBD_CtlSendData(pdev, &(haudio->tx_alt_setting), 1);
	return USBD_OK;
	break;
#endif
#ifdef USE_USBD_MIDI
      case AUDIO_MIDI_IF:
	USBD_CtlSendData(pdev, &(haudio->midi_alt_setting), 1);
	return USBD_OK;
	break;
#endif
      default:
        USBD_CtlError (pdev, req);
        ret = USBD_FAIL;
	break;
          }
          break;
        case USB_REQ_SET_INTERFACE:
          if (pdev->dev_state == USBD_STATE_CONFIGURED) {
	    switch(req->wIndex){
	    case AUDIO_RX_IF:
	      if ((uint8_t)(req->wValue) <= USBD_MAX_NUM_INTERFACES) {
		/* Do things only when alt_setting changes */
		if (haudio->rx_alt_setting != (uint8_t)(req->wValue)) {
		  haudio->rx_alt_setting = (uint8_t)(req->wValue);
		  if (haudio->rx_alt_setting == 0U) {
		    AUDIO_OUT_StopAndReset(pdev);
		  } else if (haudio->rx_alt_setting == 1U) {
		    haudio->bit_depth = 16U;
		    AUDIO_OUT_Restart(pdev);
		  } else if (haudio->rx_alt_setting == 2U) {
		    haudio->bit_depth = 24U;
		    AUDIO_OUT_Restart(pdev);
		  }
		}
		USBD_LL_FlushEP(pdev, AUDIO_FB_EP);
	      } else {
		/* Call the error management function (command will be nacked */
		USBD_CtlError(pdev, req);
		ret = USBD_FAIL;
	      }
	      break;
	    default:
	      // todo: other i/f
	      break;
	    }
          } else {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;
        default:
          USBD_CtlError(pdev, req);
          ret = USBD_FAIL;
          break;
      }
      break;
    default:
      USBD_CtlError(pdev, req);
      ret = USBD_FAIL;
      break;
  }
#else
  switch (req->bmRequest & USB_REQ_TYPE_MASK){
  case USB_REQ_TYPE_CLASS :
    if((req->bmRequest & USB_REQ_RECIPIENT_MASK) == USB_REQ_RECIPIENT_INTERFACE) {
      switch (req->bRequest) {
      case AUDIO_REQ_GET_CUR:
      case AUDIO_REQ_GET_MIN:
      case AUDIO_REQ_GET_MAX:
      case AUDIO_REQ_GET_RES:
      case AUDIO_REQ_SET_CUR:
      case AUDIO_REQ_SET_RES:
	AUDIO_REQ(pdev, req);
        break;        
      default:
        USBD_CtlError (pdev, req);
        ret = USBD_FAIL; 
      }
    }
    else
    {
     USBD_CtlError (pdev, req);
        ret = USBD_FAIL;
    }
    break;    
  case USB_REQ_TYPE_STANDARD:
    switch (req->bRequest){
    case USB_REQ_GET_DESCRIPTOR:
      USBD_DbgLog("GET_DESCRIPTOR 0x%x 0x%x 0x%x", req->wIndex, req->wValue >> 8, req->wLength);
      if( (req->wValue >> 8) == USBD_AUDIO_DESC_TYPE_CS_DEVICE){
        pbuf = USBD_AUDIO_CfgDesc + 18;
        len = MIN(USBD_AUDIO_DESC_SIZ , req->wLength);
        USBD_CtlSendData (pdev, pbuf, len);
      }
      break;
    case USB_REQ_GET_INTERFACE :
      switch(req->wIndex){
      case 0:
	USBD_CtlSendData(pdev, &(haudio->ac_alt_setting), 1);
	return USBD_OK;
	break;
#ifdef USE_USBD_AUDIO_RX
      case AUDIO_RX_IF:
	USBD_CtlSendData(pdev, &(haudio->rx_alt_setting), 1);
	return USBD_OK;
	break;
#endif
#ifdef USE_USBD_AUDIO_TX
      case AUDIO_TX_IF:
	USBD_CtlSendData(pdev, &(haudio->tx_alt_setting), 1);
	return USBD_OK;
	break;
#endif
#ifdef USE_USBD_MIDI
      case AUDIO_MIDI_IF:
	USBD_CtlSendData(pdev, &(haudio->midi_alt_setting), 1);
	return USBD_OK;
	break;
#endif
      default:
        USBD_CtlError (pdev, req);
        ret = USBD_FAIL;
	break;
      }
      break;
    case USB_REQ_SET_INTERFACE :
      return USBD_AUDIO_SetInterfaceAlternate(pdev, req);
      break;
    default:
      USBD_CtlError (pdev, req);
      ret = USBD_FAIL;     
    }
  }
#endif
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
#if 1
  /* epnum is the lowest 4 bits of bEndpointAddress. See UAC 1.0 spec, p.61 */
  if (epnum == (AUDIO_FB_EP & 0xf)) {
    tx_flag = 0U;
  }
#endif
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
      uint32_t freq __attribute__((aligned(4))) = haudio->freq;
      USBD_CtlSendData(pdev, (uint8_t*)&freq, 3);
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
      /* Frequency Control */
      if (haudio->control.cs == AUDIO_STREAMING_REQ_FREQ_CTRL) {
        uint32_t new_freq = *(uint32_t*)&haudio->control.data & 0x00ffffff;

        if (haudio->freq != new_freq) {
          haudio->freq = new_freq;
          AUDIO_OUT_Restart(pdev);
        }
      }
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
static uint8_t  USBD_AUDIO_SOF (USBD_HandleTypeDef *pdev)
{
  /* SOF (Start of Frame) Every millisecond (12000 full-bandwidth bit times), the USB host transmits a special SOF (start of frame) token, containing an 11-bit incrementing frame number in place of a device address. This is used to synchronize isochronous and interrupt data transfers. */
#if 0
  USBD_AUDIO_HandleTypeDef* haudio;
  haudio = (USBD_AUDIO_HandleTypeDef*)pdev->pClassData;
  
  /**
   * 1. Must be static so that the values are kept when the function is 
   *    again called.
   * 2. Must be volatile so that it will not be optimized out by the compiler.
   */
  // static volatile uint32_t fb_value = AUDIO_FB_DEFAULT;
  // static volatile uint32_t audio_buf_writable_size_last = AUDIO_RX_TOTAL_BUF_SIZE / 2U;
  // static volatile int32_t fb_raw = AUDIO_FB_DEFAULT;
  static volatile uint32_t sof_count = 0;

  /* Do stuff only when playing */
  if (haudio->rd_enable == 1U && all_ready == 1U) {
    /* Remaining writable buffer size */
    uint32_t audio_buf_writable_size;

    /* Update audio read pointer */
    /* haudio->rd_ptr = AUDIO_RX_TOTAL_BUF_SIZE - BSP_AUDIO_OUT_GetRemainingDataSize(); */
    /* return LL_DMA_ReadReg(AUDIO_SAIx_DMAx_STREAM, NDTR) & 0xFFFF; */
    haudio->rd_ptr = AUDIO_RX_TOTAL_BUF_SIZE/2; // todo: read HDMA_TX.Instance->NDTR
    // todo: or add usbd_audio_rx_get_read_size(), see Codec::update_rx_read_index()

    /* Calculate remaining writable buffer size */
    if (haudio->rd_ptr < haudio->wr_ptr) {
      audio_buf_writable_size = haudio->rd_ptr + AUDIO_RX_TOTAL_BUF_SIZE - haudio->wr_ptr;
    } else {
      audio_buf_writable_size = haudio->rd_ptr - haudio->wr_ptr;
    }

    /* Monitor remaining writable buffer size with LED */
    /* if (audio_buf_writable_size < AUDIO_BUF_SAFEZONE) { */
    /*   BSP_LED_On(LED2); */
    /* } else { */
    /*   BSP_LED_Off(LED2); */
    /* } */

    sof_count += 1;

    if (sof_count == 1U) {
      sof_count = 0;
      /* Calculate feedback value based on the change of writable buffer size */
      /* v2 */
      int32_t audio_buf_writable_size_dev_from_nom;
      audio_buf_writable_size_dev_from_nom = audio_buf_writable_size - (AUDIO_RX_TOTAL_BUF_SIZE >> 1);
      // fb_value += audio_buf_writable_size_dev_from_nom * 1352;
      fb_value += audio_buf_writable_size_dev_from_nom * audio_buf_writable_size_dev_from_nom / 912673 * 128 * audio_buf_writable_size_dev_from_nom;
      /* v1 */
      // int32_t audio_buf_writable_size_change;
      // audio_buf_writable_size_change = audio_buf_writable_size - audio_buf_writable_size_last;
      // fb_raw += audio_buf_writable_size_change * 0x1000;
      // fb_value = (uint32_t)fb_raw;

      /* Update last writable buffer size */
      audio_buf_writable_size_last = audio_buf_writable_size;

      /* Check feedback max / min */
      if (fb_value > fb_nom + AUDIO_FB_DELTA) {
        fb_value = fb_raw = fb_nom + AUDIO_FB_DELTA;
      } else if (fb_value < fb_nom - AUDIO_FB_DELTA) {
        fb_value = fb_raw = fb_nom - AUDIO_FB_DELTA;
      }

      /* Set 10.14 format feedback data */
      /**
       * Order of 3 bytes in feedback packet: { LO byte, MID byte, HI byte }
       * 
       * For example,
       * 48.000(dec) => 300000(hex, 8.16) => 0C0000(hex, 10.14) => packet { 00, 00, 0C }
       * 
       * Note that ALSA accepts 8.16 format.
       */
      fb_data[0] = (fb_value & 0x0000FF00) >> 8;
      fb_data[1] = (fb_value & 0x00FF0000) >> 16;
      fb_data[2] = (fb_value & 0xFF000000) >> 24;
    }

    /* Transmit feedback only when the last one is transmitted */
    if (tx_flag == 0U) {
      /* Get FNSOF. Use volatile for fnsof_new since its address is mapped to a hardware register. */
      USB_OTG_GlobalTypeDef* USBx = USB_OTG_FS;
      uint32_t USBx_BASE = (uint32_t)USBx;
      uint32_t volatile fnsof_new = (USBx_DEVICE->DSTS & USB_OTG_DSTS_FNSOF) >> 8;

      if ((fnsof & 0x1) == (fnsof_new & 0x1)) {
        USBD_LL_Transmit(pdev, AUDIO_FB_EP, (uint8_t*)fb_data, 3U);
        /* Block transmission until it's finished. */
        tx_flag = 1U;
      }
    }
  }
#else
#ifdef USE_USBD_AUDIO_FALSE // todo: Start-of-frame sync
  usbd_audio_sync_callback(0);
#endif
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
#if 1
    USB_OTG_GlobalTypeDef* USBx = USB_OTG_FS;
  uint32_t USBx_BASE = (uint32_t)USBx;
  fnsof = (USBx_DEVICE->DSTS & USB_OTG_DSTS_FNSOF) >> 8;
  if (tx_flag == 1U) {
    tx_flag = 0U;
    USBD_LL_FlushEP(pdev, AUDIO_FB_EP);
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
  USBD_AUDIO_HandleTypeDef   *haudio;
  haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;
  (void)haudio;
#ifdef USE_USBD_AUDIO_RX
  if (all_ready == 1U && epnum == AUDIO_RX_EP){
    if(is_playing == 0){
      is_playing = 1;
      if (haudio->rd_enable == 0U) {
	haudio->rd_enable = 1U;
	/* Set last writable buffer size to actual value. Note that rd_ptr is 0 now.  */
	audio_buf_writable_size_last = AUDIO_RX_TOTAL_BUF_SIZE - haudio->wr_ptr;
      }
      usbd_audio_rx_start_callback(USBD_AUDIO_FREQ, USB_AUDIO_CHANNELS);
    }
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
  }  
#endif
  return USBD_OK;
}


/**
 * @brief  Stop playing and reset buffer pointers
 * @param  pdev: instance
 */
static void AUDIO_OUT_StopAndReset(USBD_HandleTypeDef* pdev)
{
  USBD_AUDIO_HandleTypeDef* haudio;
  haudio = (USBD_AUDIO_HandleTypeDef*)pdev->pClassData;

  all_ready = 0U;
  tx_flag = 1U;
  is_playing = 0U;
  audio_buf_writable_size_last = AUDIO_RX_TOTAL_BUF_SIZE / 2U;

  haudio->offset = AUDIO_OFFSET_UNKNOWN;
  haudio->rd_enable = 0U;
  haudio->rd_ptr = 0U;
  haudio->wr_ptr = 0U;

  USBD_LL_FlushEP(pdev, AUDIO_FB_EP);
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

  AUDIO_OUT_StopAndReset(pdev);

  switch (haudio->freq) {
  case 16000:
    fb_raw = fb_nom = fb_value = 16 << 22;
    break;
  case 44100:
    fb_raw = fb_nom = fb_value = (44 << 22) + (1 << 22) / 10;
    break;
  case 48000:
    fb_raw = fb_nom = fb_value = 48 << 22;
    break;
  case 96000:
    fb_raw = fb_nom = fb_value = 96 << 22;
    break;
  default:
    fb_raw = fb_nom = fb_value = haudio->freq/1000 << 22;
  }

  /* usbd_audio_rx_start_callback(USBD_AUDIO_FREQ, USB_AUDIO_CHANNELS); */
  /* ((USBD_AUDIO_ItfTypeDef*)pdev->pUserData)->Init(haudio->freq, VOL_PERCENT(haudio->volume), 0); */

  tx_flag = 0U;
  all_ready = 1U;
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
  pdev->pUserData = fops;
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
