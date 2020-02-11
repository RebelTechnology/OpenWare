#include "device.h"
#include "Codec.h"
#include "errorhandlers.h"
#include "ApplicationSettings.h"
#include <cstring>

extern "C" {
  uint16_t codec_blocksize = 0;
  int32_t codec_txbuf[CODEC_BUFFER_SIZE];
  int32_t codec_rxbuf[CODEC_BUFFER_SIZE];
}

uint16_t Codec::getBlockSize(){
  return codec_blocksize;
}

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

void Codec::init(){
  codec_init();
}

void Codec::reset(){
  // todo: this is called when blocksize is changed
  stop();
  start();
}

void Codec::ramp(uint32_t max){
  uint32_t incr = max/CODEC_BUFFER_SIZE;
  for(int i=0; i<CODEC_BUFFER_SIZE; ++i)
    codec_txbuf[i] = i*incr;
}

void Codec::clear(){
  set(0);
}

int32_t Codec::getMin(){
  int32_t min = codec_txbuf[0];
  for(int i=1; i<CODEC_BUFFER_SIZE; ++i)
    if(codec_txbuf[i] < min)
      min = codec_txbuf[i];
  return min;
}

int32_t Codec::getMax(){
  int32_t max = codec_txbuf[0];
  for(int i=1; i<CODEC_BUFFER_SIZE; ++i)
    if(codec_txbuf[i] > max)
      max = codec_txbuf[i];
  return max;
}

float Codec::getAvg(){
  float avg = 0;
  for(int i=0; i<CODEC_BUFFER_SIZE; ++i)
    avg += codec_txbuf[i];
  return avg / CODEC_BUFFER_SIZE;
}

void Codec::set(uint32_t value){
  for(int i=0; i<CODEC_BUFFER_SIZE; ++i)
    codec_txbuf[i] = value;
}

void Codec::bypass(bool doBypass){
  codec_bypass(doBypass);
}

void Codec::mute(bool doMute){
  codec_set_gain_out(0);
}

void Codec::setInputGain(int8_t value){
  codec_set_gain_in(value);
}

void Codec::setOutputGain(int8_t value){
  codec_set_gain_out(value);
}

#ifdef USE_ADS1294
#include "ads.h"

void Codec::start(){
  codec_blocksize = AUDIO_BLOCK_SIZE;
  ads_start_continuous();
  extern TIM_HandleTypeDef htim8;
  HAL_TIM_Base_Start_IT(&htim8);
  HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_4);
}

void Codec::stop(){
  ads_stop_continuous();
}

#endif // USE_ADS1294

#ifdef USE_WM8731

extern "C" {
  extern I2S_HandleTypeDef hi2s2;
}

void Codec::stop(){
  HAL_I2S_DMAStop(&hi2s2);
}

void Codec::start(){
  setInputGain(settings.audio_input_gain);
  setOutputGain(settings.audio_output_gain);
  codec_blocksize = min(CODEC_BUFFER_SIZE/4, settings.audio_blocksize);
  HAL_StatusTypeDef ret;
  /* See STM32F405 Errata, I2S device limitations */
  /* The I2S peripheral must be enabled when the external master sets the WS line at: */
  while(HAL_GPIO_ReadPin(I2S_LRCK_GPIO_Port, I2S_LRCK_Pin)); // wait for low
  /* High level when the I2S protocol is selected. */
  while(!HAL_GPIO_ReadPin(I2S_LRCK_GPIO_Port, I2S_LRCK_Pin)); // wait for high

  // When a 16-bit data frame or a 16-bit data frame extended is selected during the I2S
  // configuration phase, the Size parameter means the number of 16-bit data length
  // in the transaction and when a 24-bit data frame or a 32-bit data frame is selected
  // the Size parameter means the number of 16-bit data length.
  ret = HAL_I2SEx_TransmitReceive_DMA(&hi2s2, (uint16_t*)codec_txbuf, (uint16_t*)codec_rxbuf, codec_blocksize*4);
  ASSERT(ret == HAL_OK, "Failed to start I2S DMA");
}

void Codec::pause(){
  HAL_I2S_DMAPause(&hi2s2);
}

void Codec::resume(){
  HAL_I2S_DMAResume(&hi2s2);
}

extern "C"{
  
  void HAL_I2SEx_TxRxHalfCpltCallback(I2S_HandleTypeDef *hi2s){
    audioCallback(codec_rxbuf, codec_txbuf, codec_blocksize);
  }

  void HAL_I2SEx_TxRxCpltCallback(I2S_HandleTypeDef *hi2s){
    audioCallback(codec_rxbuf+codec_blocksize*2, codec_txbuf+codec_blocksize*2, codec_blocksize);
  }

  void HAL_I2S_ErrorCallback(I2S_HandleTypeDef *hi2s){
    error(CONFIG_ERROR, "I2S Error");
  }
  
}
#endif /* USE_WM8731 */

#if defined USE_CS4271 || defined USE_PCM3168A

extern "C" {
  SAI_HandleTypeDef hsai_BlockA1;
  SAI_HandleTypeDef hsai_BlockB1;
  void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai){
    audioCallback(codec_rxbuf, codec_txbuf, codec_blocksize);
  }
  void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai){
    audioCallback(codec_rxbuf+codec_blocksize*AUDIO_CHANNELS, codec_txbuf+codec_blocksize*AUDIO_CHANNELS, codec_blocksize);
  }
  void HAL_SAI_ErrorCallback(SAI_HandleTypeDef *hsai){
    error(CONFIG_ERROR, "SAI DMA Error");
  }
}

void Codec::txrx(){
  HAL_SAI_DMAStop(&hsai_BlockA1);
  HAL_SAI_Transmit_DMA(&hsai_BlockB1, (uint8_t*)codec_rxbuf, codec_blocksize*AUDIO_CHANNELS*2);
}

void Codec::stop(){
  HAL_SAI_DMAStop(&hsai_BlockA1);
  HAL_SAI_DMAStop(&hsai_BlockB1);
}

void Codec::start(){
  setOutputGain(settings.audio_output_gain);
  codec_blocksize = min(CODEC_BUFFER_SIZE/(AUDIO_CHANNELS*2), settings.audio_blocksize);
  HAL_StatusTypeDef ret;
#ifdef USE_CS4271
  ret = HAL_SAI_Receive_DMA(&hsai_BlockB1, (uint8_t*)codec_rxbuf, codec_blocksize*AUDIO_CHANNELS*2);
  ASSERT(ret == HAL_OK, "Failed to start SAI RX DMA");
  ret = HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t*)codec_txbuf, codec_blocksize*AUDIO_CHANNELS*2);
  ASSERT(ret == HAL_OK, "Failed to start SAI TX DMA");
#else
  ret = HAL_SAI_Receive_DMA(&hsai_BlockA1, (uint8_t*)codec_rxbuf, codec_blocksize*AUDIO_CHANNELS*2);
  ASSERT(ret == HAL_OK, "Failed to start SAI RX DMA");
  ret = HAL_SAI_Transmit_DMA(&hsai_BlockB1, (uint8_t*)codec_txbuf, codec_blocksize*AUDIO_CHANNELS*2);
  ASSERT(ret == HAL_OK, "Failed to start SAI TX DMA");
#endif
}

void Codec::pause(){
  HAL_SAI_DMAPause(&hsai_BlockB1);
  HAL_SAI_DMAPause(&hsai_BlockA1);
}

void Codec::resume(){
  HAL_SAI_DMAResume(&hsai_BlockB1);
  HAL_SAI_DMAResume(&hsai_BlockA1);
}

extern "C" {

// void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai){
// }

}

#endif /* USE_PCM3168A */

