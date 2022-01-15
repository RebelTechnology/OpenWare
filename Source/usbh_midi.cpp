#include "usbh_midi.h"
#include "usb_host.h"
#include "midi.h"
#include "SerialBuffer.hpp"

/**
 * USB Host MIDI Driver
 * Based on code by Xavier Halgand @MrBlueXav
 */

extern "C" {
  static USBH_StatusTypeDef USBH_MIDI_InterfaceInit  (USBH_HandleTypeDef *phost);

  static USBH_StatusTypeDef USBH_MIDI_InterfaceDeInit  (USBH_HandleTypeDef *phost);

  static USBH_StatusTypeDef USBH_MIDI_Process(USBH_HandleTypeDef *phost);

  static USBH_StatusTypeDef USBH_MIDI_SOFProcess(USBH_HandleTypeDef *phost);

  static USBH_StatusTypeDef USBH_MIDI_ClassRequest (USBH_HandleTypeDef *phost);

  static void MIDI_ProcessTransmission(USBH_HandleTypeDef *phost);

  USBH_ClassTypeDef  MIDI_Class =
    {
      "MIDI", // Name
      USB_AUDIO_CLASS, // ClassCode
      USBH_MIDI_InterfaceInit, // Init
      USBH_MIDI_InterfaceDeInit, // DeInit
      USBH_MIDI_ClassRequest, // Requests
      USBH_MIDI_Process, // BgndProcess: background process called in HOST_CLASS state (core state machine)
      USBH_MIDI_SOFProcess, // SOFProcess
      NULL // pData: MIDI handle structure
    };

  void usbh_midi_push();
}

static SerialBuffer<USB_HOST_RX_BUFF_SIZE> rxbuffer;
static MIDI_HandleTypeDef staticMidiHandle;

static USBH_StatusTypeDef USBH_MIDI_InterfaceInit (USBH_HandleTypeDef *phost){
  USBH_StatusTypeDef status = USBH_FAIL ;
  //USB_MIDI_ChangeConnectionState(0);

  uint8_t interface = USBH_FindInterface(phost, USB_AUDIO_CLASS, USB_MIDISTREAMING_SUBCLASS, 0xFF);
  USBH_DbgLog ("USBH InterfaceInit 0x%x", interface);

  if(interface == 0xFF){
    /* No Valid Interface */
    USBH_DbgLog ("Cannot Find the interface for MIDI Interface Class %s.", phost->pActiveClass->Name);
    status = USBH_FAIL;
  }else{
    USBH_SelectInterface (phost, interface);
    MIDI_HandleTypeDef* MIDI_Handle = &staticMidiHandle;
    memset(MIDI_Handle, 0, sizeof(staticMidiHandle));
    phost->pActiveClass->pData = MIDI_Handle;

    if(phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[0].bEndpointAddress & 0x80){
      MIDI_Handle->InEp = (phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[0].bEndpointAddress);
      MIDI_Handle->InEpSize  = phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[0].wMaxPacketSize;
    }else{
      MIDI_Handle->OutEp = (phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[0].bEndpointAddress);
      MIDI_Handle->OutEpSize  = phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[0].wMaxPacketSize;
    }

    if(phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[1].bEndpointAddress & 0x80){
      MIDI_Handle->InEp = (phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[1].bEndpointAddress);
      MIDI_Handle->InEpSize  = phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[1].wMaxPacketSize;
    }else{
      MIDI_Handle->OutEp = (phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[1].bEndpointAddress);
      MIDI_Handle->OutEpSize  = phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[1].wMaxPacketSize;
    }

    MIDI_Handle->OutPipe = USBH_AllocPipe(phost, MIDI_Handle->OutEp);
    MIDI_Handle->InPipe = USBH_AllocPipe(phost, MIDI_Handle->InEp);


    /* Open the new channels */
    USBH_OpenPipe  (phost,
		    MIDI_Handle->OutPipe,
		    MIDI_Handle->OutEp,
		    phost->device.address,
		    phost->device.speed,
		    USB_EP_TYPE_BULK,
		    MIDI_Handle->OutEpSize);

    USBH_OpenPipe  (phost,
		    MIDI_Handle->InPipe,
		    MIDI_Handle->InEp,
		    phost->device.address,
		    phost->device.speed,
		    USB_EP_TYPE_BULK,
		    MIDI_Handle->InEpSize);

    //USB_MIDI_ChangeConnectionState(1);
    MIDI_Handle->state = MIDI_IDLE_STATE;


    USBH_LL_SetToggle  (phost, MIDI_Handle->InPipe,0);
    USBH_LL_SetToggle  (phost, MIDI_Handle->OutPipe,0);

    status = USBH_OK;
  }
  return status;
}

/**
 * @brief  USBH_MIDI_InterfaceDeInit
 *         The function DeInit the Pipes used for the MIDI class.
 * @param  phost: Host handle
 * @retval USBH Status
 */
USBH_StatusTypeDef USBH_MIDI_InterfaceDeInit (USBH_HandleTypeDef *phost){
  USBH_DbgLog ("USBH InterfaceDeInit");
  if(phost->pActiveClass->pData){
    MIDI_HandleTypeDef* MIDI_Handle = (MIDI_HandleTypeDef*)phost->pActiveClass->pData;

    if ( MIDI_Handle->OutPipe){
      USBH_ClosePipe(phost, MIDI_Handle->OutPipe);
      USBH_FreePipe  (phost, MIDI_Handle->OutPipe);
      MIDI_Handle->OutPipe = 0;     /* Reset the Channel as Free */
    }
    
    if ( MIDI_Handle->InPipe){
      USBH_ClosePipe(phost, MIDI_Handle->InPipe);
      USBH_FreePipe  (phost, MIDI_Handle->InPipe);
      MIDI_Handle->InPipe = 0;     /* Reset the Channel as Free */
    }

    /* statically allocated
       USBH_free (phost->pActiveClass->pData); */
    phost->pActiveClass->pData = NULL;
  }

  return USBH_OK;
}

/**
 * @brief  USBH_MIDI_ClassRequest
 *         The function is responsible for handling Standard requests
 *         for MIDI class.
 * @param  phost: Host handle
 * @retval USBH Status
 */
static USBH_StatusTypeDef USBH_MIDI_ClassRequest (USBH_HandleTypeDef *phost){
  phost->pUser(phost, HOST_USER_CLASS_ACTIVE);
  return USBH_OK;
}

/**
  * @brief  USBH_MIDI_Stop
  *         Stop current MIDI Transmission
  * @param  phost: Host handle
  * @retval USBH Status
  */
USBH_StatusTypeDef  USBH_MIDI_Stop(USBH_HandleTypeDef *phost){
  USBH_DbgLog ("USBH Stop 0x%x", phost->gState);
  MIDI_HandleTypeDef *MIDI_Handle =  &staticMidiHandle;
  if(phost->gState == HOST_CLASS){
    MIDI_Handle->state = MIDI_IDLE_STATE;
    USBH_ClosePipe(phost, MIDI_Handle->InPipe);
    USBH_ClosePipe(phost, MIDI_Handle->OutPipe);
  }
  return USBH_OK;
}

extern "C"{
  void HAL_HCD_HC_NotifyURBChange_Callback(HCD_HandleTypeDef *hhcd, uint8_t chnum, HCD_URBStateTypeDef urb_state){
    MIDI_HandleTypeDef *MIDI_Handle =  &staticMidiHandle;
    if(urb_state == URB_DONE && chnum == MIDI_Handle->InPipe &&
       MIDI_Handle->state == MIDI_TRANSFER_DATA){
      size_t len = USBH_LL_GetLastXferSize((USBH_HandleTypeDef*)hhcd->pData, MIDI_Handle->InPipe);
      USBH_MIDI_ReceiveCallback((USBH_HandleTypeDef*)hhcd->pData, MIDI_Handle->pRxData, len);
    }
  }
}

/**
 * @brief  USBH_MIDI_Process
 *         The function is for managing state machine for MIDI data transfers
 *         (background process)
 * @param  phost: Host handle
 * @retval USBH Status
 */
static USBH_StatusTypeDef USBH_MIDI_Process (USBH_HandleTypeDef *phost){
  USBH_StatusTypeDef status = USBH_BUSY;
  USBH_StatusTypeDef req_status = USBH_OK;
  MIDI_HandleTypeDef *MIDI_Handle =  &staticMidiHandle;
  switch(MIDI_Handle->state){
  case MIDI_IDLE_STATE:
    status = USBH_OK;
    break;
  case MIDI_TRANSFER_DATA:
    MIDI_ProcessTransmission(phost);
    break;
  case MIDI_ERROR_STATE:
    req_status = USBH_ClrFeature(phost, 0x00);
    if(req_status == USBH_OK){
      /*Change the state to waiting*/
      MIDI_Handle->state = MIDI_IDLE_STATE ;
    }
    break;
  default:
    break;
  }
  return status;
}

/**
  * @brief  USBH_MIDI_SOFProcess 
  *         The function is for managing SOF callback 
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_MIDI_SOFProcess (USBH_HandleTypeDef *phost){
  MIDI_HandleTypeDef *MIDI_Handle =  &staticMidiHandle;

  USBH_URBStateTypeDef URB_Status = USBH_LL_GetURBState(phost, MIDI_Handle->InPipe);
  if(URB_Status == USBH_URB_STALL) {
    USBH_DbgLog("USBH URB Stall");
    if (USBH_ClrFeature(phost, MIDI_Handle->InEp) == USBH_OK)
      MIDI_Handle->state = MIDI_TRANSFER_DATA;
  }
  return USBH_OK;  
}
  
/**
 * @brief  This function return last recieved data size
 * @param  None
 * @retval None
 */
uint16_t USBH_MIDI_GetLastReceivedDataSize(USBH_HandleTypeDef *phost){
  MIDI_HandleTypeDef *MIDI_Handle =  &staticMidiHandle;
  if(phost->gState == HOST_CLASS)
    return USBH_LL_GetLastXferSize(phost, MIDI_Handle->InPipe);
  else
    return 0;
}

/**
 * @brief  This function prepares the state before issuing the class specific commands
 * @param  None
 * @retval None
 */
USBH_StatusTypeDef  USBH_MIDI_Transmit(USBH_HandleTypeDef *phost, uint8_t *pbuff, uint16_t length){
  USBH_StatusTypeDef Status = USBH_BUSY;
  MIDI_HandleTypeDef *MIDI_Handle =  &staticMidiHandle;

  if((MIDI_Handle->state == MIDI_IDLE_STATE) || (MIDI_Handle->state == MIDI_TRANSFER_DATA))
    {
      MIDI_Handle->pTxData = pbuff;
      MIDI_Handle->TxDataLength = length;
      MIDI_Handle->state = MIDI_TRANSFER_DATA;
      MIDI_Handle->data_tx_state = MIDI_SEND_DATA;
      Status = USBH_OK;
#if (USBH_USE_OS == 1)
      osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
    }
  return Status;
}

/**
 * @brief  This function prepares the state before issuing the class specific commands
 * @param  None
 * @retval None
 */
USBH_StatusTypeDef  USBH_MIDI_Receive(USBH_HandleTypeDef *phost, uint8_t *pbuff, uint16_t length){
  USBH_StatusTypeDef Status = USBH_BUSY;
  MIDI_HandleTypeDef *MIDI_Handle =  &staticMidiHandle;

    if((MIDI_Handle->state == MIDI_IDLE_STATE) || (MIDI_Handle->state == MIDI_TRANSFER_DATA)){
      MIDI_Handle->pRxData = pbuff;
      MIDI_Handle->RxDataLength = length;
      MIDI_Handle->state = MIDI_TRANSFER_DATA;
      USBH_BulkReceiveData(phost,
			   MIDI_Handle->pRxData,
			   MIDI_Handle->InEpSize,
			   MIDI_Handle->InPipe);
      Status = USBH_OK;
#if (USBH_USE_OS == 1)
      osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
    }
  return Status;
}

/**
 * @brief  The function is responsible for sending data to the device
 *  @param  pdev: Selected device
 * @retval None
 */
static void MIDI_ProcessTransmission(USBH_HandleTypeDef *phost){
  MIDI_HandleTypeDef *MIDI_Handle =  &staticMidiHandle;
  USBH_URBStateTypeDef URB_Status = USBH_URB_IDLE;

  switch(MIDI_Handle->data_tx_state)
    {

    case MIDI_SEND_DATA:
      if(MIDI_Handle->TxDataLength > MIDI_Handle->OutEpSize)
	{
	  USBH_BulkSendData (phost,
			     MIDI_Handle->pTxData,
			     MIDI_Handle->OutEpSize,
			     MIDI_Handle->OutPipe,
			     1);
	}
      else
	{
	  USBH_BulkSendData (phost,
			     MIDI_Handle->pTxData,
			     MIDI_Handle->TxDataLength,
			     MIDI_Handle->OutPipe,
			     1);
	}

      MIDI_Handle->data_tx_state = MIDI_SEND_DATA_WAIT;

      break;

    case MIDI_SEND_DATA_WAIT:

      URB_Status = USBH_LL_GetURBState(phost, MIDI_Handle->OutPipe);

      /*Check the status done for transmission*/
      if(URB_Status == USBH_URB_DONE )
	{
	  if(MIDI_Handle->TxDataLength > MIDI_Handle->OutEpSize)
	    {
	      MIDI_Handle->TxDataLength -= MIDI_Handle->OutEpSize ;
	      MIDI_Handle->pTxData += MIDI_Handle->OutEpSize;
	    }
	  else
	    {
	      MIDI_Handle->TxDataLength = 0;
	    }

	  if( MIDI_Handle->TxDataLength > 0)
	    {
	      MIDI_Handle->data_tx_state = MIDI_SEND_DATA;
	    }
	  else
	    {
	      MIDI_Handle->data_tx_state = MIDI_IDLE;
	      USBH_MIDI_TransmitCallback(phost);
	    }
#if (USBH_USE_OS == 1)
	  osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
	}
      else if( URB_Status == USBH_URB_NOTREADY )
	{
	  MIDI_Handle->data_tx_state = MIDI_SEND_DATA;
#if (USBH_USE_OS == 1)
	  osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
	}
      break;
    default:
      break;
    }
}

__weak void usbh_midi_reset(void){
  // extern USBH_HandleTypeDef USBH_HANDLE; // defined in usb_host.c
  // USBH_LL_ResetPort(&USBH_HANDLE);
}

bool usbh_midi_connected(void){
  extern USBH_HandleTypeDef USBH_HANDLE; // defined in usb_host.c
  return USBH_HANDLE.device.is_connected;
  // extern ApplicationTypeDef Appli_state; // defined in usb_host.c
  // return Appli_state == APPLICATION_START || Appli_state == APPLICATION_READY;
}

bool usbh_midi_ready(void){
  extern ApplicationTypeDef Appli_state;
  extern USBH_HandleTypeDef USBH_HANDLE; // defined in usb_host.c
  USBH_ClassTypeDef* activeClass = USBH_HANDLE.pActiveClass;
  if(Appli_state == APPLICATION_READY && activeClass != NULL && activeClass->pData){
    MIDI_HandleTypeDef *MIDI_Handle = (MIDI_HandleTypeDef*)activeClass->pData;
    return MIDI_Handle != NULL && MIDI_Handle->data_tx_state == MIDI_IDLE;
  }
  return false;
}

void USBH_MIDI_ReceiveCallback(USBH_HandleTypeDef *phost, uint8_t* data, size_t len){
  // len is always 64 at this point, even if only 4 bytes of data is transferred
  // it appears some drivers (e.g. WinXP) send 0-padded 64-length packets
  size_t sz = 0;
  while(sz<len && data[sz] != 0)
    sz += 4;
  rxbuffer.incrementWriteHead(sz);
  USBH_MIDI_Receive(phost, rxbuffer.getWriteHead(), rxbuffer.getContiguousWriteCapacity());
}

void usbh_midi_begin(){
  extern USBH_HandleTypeDef USBH_HANDLE; // defined in usb_host.c
  USBH_MIDI_Receive(&USBH_HANDLE, rxbuffer.getWriteHead(), rxbuffer.getContiguousWriteCapacity());
}

void usbh_midi_tx(uint8_t* buffer, uint32_t length){
  extern USBH_HandleTypeDef USBH_HANDLE; // defined in usb_host.c
  USBH_MIDI_Transmit(&USBH_HANDLE, buffer, length);
}

void USBH_MIDI_TransmitCallback(USBH_HandleTypeDef *phost){
  // tx complete callback
// get ready to send some data
}

void usbh_midi_push(){
  if(rxbuffer.notEmpty()){
    size_t len = rxbuffer.getContiguousReadCapacity();
    usbh_midi_rx(rxbuffer.getReadHead(), len);
    rxbuffer.incrementReadHead(len);
  }
}
