#ifndef __BSP_AUDIO_H
#define __BSP_AUDIO_H

#ifdef __cplusplus
extern "C" {
#endif

uint8_t BSP_AUDIO_IN_Init(uint16_t OutputDevice, uint8_t Volume, uint32_t AudioFreq);
uint8_t BSP_AUDIO_IN_Play(int16_t* pBuffer, uint32_t Size);

uint8_t BSP_AUDIO_IN_SetVolume(uint8_t Volume);
uint8_t BSP_AUDIO_IN_SetMute(uint32_t Cmd);
uint8_t BSP_AUDIO_IN_Stop(uint32_t Option);


uint8_t BSP_AUDIO_OUT_Init(uint16_t OutputDevice, uint8_t Volume, uint32_t AudioFreq);
void    BSP_AUDIO_OUT_DeInit(void);
uint8_t BSP_AUDIO_OUT_Play(uint16_t* pBuffer, uint32_t Size);
void    BSP_AUDIO_OUT_ChangeBuffer(uint16_t *pData, uint16_t Size);
uint8_t BSP_AUDIO_OUT_Pause(void);
uint8_t BSP_AUDIO_OUT_Resume(void);
uint8_t BSP_AUDIO_OUT_Stop(uint32_t Option);
uint8_t BSP_AUDIO_OUT_SetVolume(uint8_t Volume);
void    BSP_AUDIO_OUT_SetFrequency(uint32_t AudioFreq);
void    BSP_AUDIO_OUT_SetAudioFrameSlot(uint32_t AudioFrameSlot);
uint8_t BSP_AUDIO_OUT_SetMute(uint32_t Cmd);
uint8_t BSP_AUDIO_OUT_SetOutputMode(uint8_t Output);

/* User Callbacks: user has to implement these functions in his code if they are needed. */
/* This function is called when the requested data has been completely transferred.*/
void    BSP_AUDIO_OUT_TransferComplete_CallBack(void);

/* This function is called when half of the requested buffer has been transferred. */
void    BSP_AUDIO_OUT_HalfTransfer_CallBack(void);

/* This function is called when an Interrupt due to transfer error on or peripheral
   error occurs. */
void    BSP_AUDIO_OUT_Error_CallBack(void);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_AUDIO_H */
