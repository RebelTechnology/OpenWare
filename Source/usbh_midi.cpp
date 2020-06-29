#include "usbh_midi.h"
#include "usb_host.h"
#include "midi.h"
#include "SerialBuffer.hpp"

extern "C" {
  static USBH_StatusTypeDef USBH_MIDI_InterfaceInit  (USBH_HandleTypeDef *phost);

  static USBH_StatusTypeDef USBH_MIDI_InterfaceDeInit  (USBH_HandleTypeDef *phost);

  static USBH_StatusTypeDef USBH_MIDI_Process(USBH_HandleTypeDef *phost);

  static USBH_StatusTypeDef USBH_MIDI_SOFProcess(USBH_HandleTypeDef *phost);

  static USBH_StatusTypeDef USBH_MIDI_ClassRequest (USBH_HandleTypeDef *phost);

  static void MIDI_ProcessTransmission(USBH_HandleTypeDef *phost);

  static void MIDI_ProcessReception(USBH_HandleTypeDef *phost);

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
  uint8_t interface = 0;
  MIDI_HandleTypeDef *MIDI_Handle;

  //USB_MIDI_ChangeConnectionState(0);

  interface = USBH_FindInterface(phost, USB_AUDIO_CLASS, USB_MIDISTREAMING_SUBCLASS, 0xFF);

  if(interface == 0xFF){
    /* No Valid Interface */
    USBH_DbgLog ("Cannot Find the interface for MIDI Interface Class.", phost->pActiveClass->Name);
    status = USBH_FAIL;
  }else{
    USBH_SelectInterface (phost, interface);
    phost->pActiveClass->pData = &staticMidiHandle;
    MIDI_Handle =  &staticMidiHandle;

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
  MIDI_HandleTypeDef *MIDI_Handle =  &staticMidiHandle;

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

  if(phost->pActiveClass->pData){
    /* statically allocated in init
       USBH_free (phost->pActiveClass->pData); */
    phost->pActiveClass->pData = 0;
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
  MIDI_HandleTypeDef *MIDI_Handle =  &staticMidiHandle;
  if(phost->gState == HOST_CLASS){
    MIDI_Handle->state = MIDI_IDLE_STATE;
    USBH_ClosePipe(phost, MIDI_Handle->InPipe);
    USBH_ClosePipe(phost, MIDI_Handle->OutPipe);
  }
  return USBH_OK;
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
    MIDI_ProcessReception(phost);
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

  if(MIDI_Handle == NULL){
    Status = USBH_FAIL;
  }else if((MIDI_Handle->state == MIDI_IDLE_STATE) || (MIDI_Handle->state == MIDI_TRANSFER_DATA))
    {
      MIDI_Handle->pRxData = pbuff;
      MIDI_Handle->RxDataLength = length;
      MIDI_Handle->state = MIDI_TRANSFER_DATA;
      MIDI_Handle->data_rx_state = MIDI_RECEIVE_DATA;
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

/**
 * @brief  This function responsible for reception of data from the device
 *  @param  pdev: Selected device
 * @retval None
 */
static void MIDI_ProcessReception(USBH_HandleTypeDef *phost){
  MIDI_HandleTypeDef *MIDI_Handle =  &staticMidiHandle;
  USBH_URBStateTypeDef URB_Status = USBH_URB_IDLE;
  uint16_t length;

  switch(MIDI_Handle->data_rx_state){

  case MIDI_RECEIVE_DATA:

    USBH_BulkReceiveData (phost,
			  MIDI_Handle->pRxData,
			  MIDI_Handle->InEpSize,
			  MIDI_Handle->InPipe);

    MIDI_Handle->data_rx_state = MIDI_RECEIVE_DATA_WAIT;
    //BSP_LED_On(LED_Red); //ok only here

    break;

  case MIDI_RECEIVE_DATA_WAIT:

    URB_Status = USBH_LL_GetURBState(phost, MIDI_Handle->InPipe);



    /*Check the status done for reception*/
    if(URB_Status == USBH_URB_DONE ){
      length = USBH_LL_GetLastXferSize(phost, MIDI_Handle->InPipe);
      if(((MIDI_Handle->RxDataLength - length) > 0) && (length > MIDI_Handle->InEpSize)){
	MIDI_Handle->RxDataLength -= length ;
	MIDI_Handle->pRxData += length;
	MIDI_Handle->data_rx_state = MIDI_RECEIVE_DATA;
      }else{
	MIDI_Handle->data_rx_state = MIDI_IDLE;
	USBH_MIDI_ReceiveCallback(phost);
      }
#if (USBH_USE_OS == 1)
      osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
    }
    break;

  default:
    break;
  }
}

bool usbh_midi_connected(void){
  extern ApplicationTypeDef Appli_state; // defined in usb_host.c
  return Appli_state == APPLICATION_START || Appli_state == APPLICATION_READY;
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

void USBH_MIDI_ReceiveCallback(USBH_HandleTypeDef *phost){
  // extern uint8_t USB_HOST_RX_BUFFER[USB_HOST_RX_BUFF_SIZE];
  size_t len = USBH_MIDI_GetLastReceivedDataSize(phost);
  rxbuffer.incrementWriteHead(len);
  // len = rxbuffer.getContiguousWriteCapacity();
  // usbh_midi_rx(USB_HOST_RX_BUFFER, len);
  // USBH_MIDI_Receive(phost, USB_HOST_RX_BUFFER, USB_HOST_RX_BUFF_SIZE); // start a new reception
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
// get ready to send some data
}

void usbh_midi_push(){
  if(rxbuffer.notEmpty()){
    size_t len = rxbuffer.getContiguousReadCapacity();
    usbh_midi_rx(rxbuffer.getReadHead(), len);
    rxbuffer.incrementReadHead(len);
  }
}
