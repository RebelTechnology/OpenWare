#ifndef __USBH_MIDI_CORE_H
#define __USBH_MIDI_CORE_H

#include "usbh_core.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define USB_MIDI_DATA_OUT_EP        0x02
#define USB_MIDI_DATA_IN_EP         0x81
#define USB_AUDIO_CLASS             0x01
#define USB_MIDISTREAMING_SUBCLASS  0x03
#define USB_MIDI_DESC_SIZE          9
#define USBH_MIDI_CLASS             &MIDI_Class

  /*-------------------------------------------------------------------------------*/

  extern USBH_ClassTypeDef  MIDI_Class;

  typedef enum {
    Chn1,
    Chn2,
    Chn3,
    Chn4,
    Chn5,
    Chn6,
    Chn7,
    Chn8,
    Chn9,
    Chn10,
    Chn11,
    Chn12,
    Chn13,
    Chn14,
    Chn15,
    Chn16
  } midi_chn_t;

  /* States for MIDI State Machine */
  typedef enum {
    MIDI_IDLE= 0,
    MIDI_SEND_DATA,
    MIDI_SEND_DATA_WAIT,
    MIDI_RECEIVE_DATA,
    MIDI_RECEIVE_DATA_WAIT,
  } MIDI_DataStateTypeDef;
    
  typedef enum {
    MIDI_IDLE_STATE= 0,
    MIDI_TRANSFER_DATA,
    MIDI_ERROR_STATE,
  } MIDI_StateTypeDef;    

  /* Structure for MIDI process */
  typedef struct _MIDI_Process {
    MIDI_StateTypeDef			state;
    uint8_t			InPipe;
    uint8_t			OutPipe;
    uint8_t			OutEp;
    uint8_t			InEp;
    uint16_t		OutEpSize;
    uint16_t		InEpSize;

    uint8_t			*pTxData;
    uint8_t			*pRxData;
    uint16_t		TxDataLength;
    uint16_t		RxDataLength;
    MIDI_DataStateTypeDef		data_tx_state;
    MIDI_DataStateTypeDef		data_rx_state;
    uint8_t						Rx_Poll;
    //uint8_t			buff[8];
    //MIDI_DataItfTypedef                DataItf;
    //CDC_InterfaceDesc_Typedef         CDC_Desc;
  } MIDI_HandleTypeDef;

  void usbh_midi_begin(void);
  void usbh_midi_push(void);
  void usbh_midi_reset(void);
  bool usbh_midi_connected(void);
  bool usbh_midi_ready(void);

  USBH_StatusTypeDef  USBH_MIDI_Transmit(USBH_HandleTypeDef *phost,
					 uint8_t *pbuff,
					 uint16_t length);

  USBH_StatusTypeDef  USBH_MIDI_Receive(USBH_HandleTypeDef *phost,
					uint8_t *pbuff,
					uint16_t length);


  uint16_t            USBH_MIDI_GetLastReceivedDataSize(USBH_HandleTypeDef *phost);

  USBH_StatusTypeDef  USBH_MIDI_Stop(USBH_HandleTypeDef *phost);

  void USBH_MIDI_TransmitCallback(USBH_HandleTypeDef *phost);

  void USBH_MIDI_ReceiveCallback(USBH_HandleTypeDef *phost);

#ifdef __cplusplus
}
#endif

#endif /* __USBH_MIDI_CORE_H */

