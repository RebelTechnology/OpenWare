#include <stdint.h>
#include "bsp_audio.h"
#include "stm32f4xx_hal.h"
#include "usbd_audio.h"


// #define ADC_RINGBUFFER_SIZE 4096
// static int16_t ringbuffer_data[ADC_RINGBUFFER_SIZE];
// static RingBuffer<int16_t> ringbuffer(ringbuffer_data, ADC_RINGBUFFER_SIZE);
// // static int16_t mic_data[AUDIO_IN_PACKET_SIZE];

// void adc_rx_callback(int16_t* data, size_t len){
//   ringbuffer.write(data, len);
// }

uint8_t BSP_AUDIO_IN_SetMute(uint32_t Cmd){
    return 0;
}
uint8_t BSP_AUDIO_IN_SetVolume(uint8_t Volume){
    return 0;
}

/* uint16_t frame=0; */
/* int16_t mic_data[AUDIO_IN_PACKET_SIZE]; */

/* uint8_t audio_device_ready(void){ */
/*   return audio_tx_lock == 0; */
/* } */

/* void usb_audio_tx(uint8_t* buf, uint32_t len) { */
/*   extern USBD_HandleTypeDef hUsbDeviceFS; */
/*   if(hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED){ */
/*     while(audio_tx_lock); */
/*     audio_tx_lock = 1; */
/*     USBD_LL_Transmit(&hUsbDeviceFS, AUDIO_IN_EP, buf, len); */
/*   } */
/* } */

uint8_t BSP_AUDIO_IN_Play(int16_t* pBuffer, uint32_t Size){
  // usbd_audio_rxne();
  // ringbuffer.read(pBuffer, Size, true);
  return 0;
}

uint8_t BSP_AUDIO_IN_Init(uint16_t OutputDevice, uint8_t Volume, uint32_t AudioFreq){

  /* Initialize fake microphone buffer with ramp data. */    
    /* for(frame=0; frame<AUDIO_IN_PACKET_SIZE;frame++) */
    /*   mic_data[frame] = (UINT16_MAX * (int32_t)frame)/AUDIO_IN_PACKET_SIZE - INT16_MAX; */
    /* for(frame=0; frame<AUDIO_IN_PACKET_SIZE;frame++) */
    /*   mic_data[frame] *= (frame % 2) ? 1 : -1; */

/* static volatile int audio_tx_lock = 0; */
  return 0;
}


uint8_t BSP_AUDIO_OUT_Init(uint16_t OutputDevice, uint8_t Volume, uint32_t AudioFreq){
  /* uint8_t ret = AUDIO_ERROR; */
  /* ret = AUDIO_OK; */
  /* // configure sample rate and gain */
  /* return ret; */
  return 0;
}

void    BSP_AUDIO_OUT_DeInit(void){
}

uint8_t BSP_AUDIO_OUT_Play(uint16_t* pBuffer, uint32_t Size){
// start DMA transfer
  return 0;
}

/**
  * @brief  Sends n-Bytes on the SAI interface.
  * @param  pData: pointer on data address
  * @param  Size: number of data to be written
  * @retval None
  */
void    BSP_AUDIO_OUT_ChangeBuffer(uint16_t *pData, uint16_t Size){
  // USBD_DbgLog("change buffer");
}
								   
/**
  * @brief  This function Pauses the audio file stream. In case
  *         of using DMA, the DMA Pause feature is used.
  * @warning When calling BSP_AUDIO_OUT_Pause() function for pause, only
  *          BSP_AUDIO_OUT_Resume() function should be called for resume (use of BSP_AUDIO_OUT_Play()
  *          function for resume could lead to unexpected behaviour).
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
uint8_t BSP_AUDIO_OUT_Pause(void){
    return 0;
}
uint8_t BSP_AUDIO_OUT_Resume(void){
    return 0;
}
uint8_t BSP_AUDIO_OUT_Stop(uint32_t Option){
    return 0;
}
uint8_t BSP_AUDIO_OUT_SetVolume(uint8_t Volume){
    return 0;
}
void    BSP_AUDIO_OUT_SetFrequency(uint32_t AudioFreq){
}
void    BSP_AUDIO_OUT_SetAudioFrameSlot(uint32_t AudioFrameSlot){
}
uint8_t BSP_AUDIO_OUT_SetMute(uint32_t Cmd){
    return 0;
}
uint8_t BSP_AUDIO_OUT_SetOutputMode(uint8_t Output){
    return 0;
}

/* User Callbacks: user has to implement these functions in his code if they are needed. */
/* This function is called when the requested data has been completely transferred.*/
void    BSP_AUDIO_OUT_TransferComplete_CallBack(void);

/* This function is called when half of the requested buffer has been transferred. */
void    BSP_AUDIO_OUT_HalfTransfer_CallBack(void);

/* This function is called when an Interrupt due to transfer error on or peripheral
   error occurs. */
void    BSP_AUDIO_OUT_Error_CallBack(void);
