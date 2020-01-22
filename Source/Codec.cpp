#include "device.h"
#include "Codec.h"
#include "errorhandlers.h"
#include "ApplicationSettings.h"
#include <cstring>

#if AUDIO_BITS_PER_SAMPLE == 32
typedef int32_t audio_t;
#elif AUDIO_BITS_PER_SAMPLE == 16
typedef int16_t audio_t;
#elif AUDIO_BITS_PER_SAMPLE == 8
typedef int8_t audio_t;
#else
#error Invalid AUDIO_BITS_PER_SAMPLE
#endif

#include "SerialBuffer.hpp"
SerialBuffer<CODEC_BUFFER_SIZE, int32_t> audio_rx_buffer;

extern "C" {
  uint16_t codec_blocksize = 0;
  int32_t codec_rxbuf[CODEC_BUFFER_SIZE];
  int32_t* codec_txbuf; // todo: ensure buffer alignment
}

#ifdef USE_USB_AUDIO
#include "usbd_audio.h"
volatile static size_t adc_underflow = 0;
SerialBuffer<AUDIO_RINGBUFFER_SIZE, audio_t> audio_tx_buffer;

void usbd_audio_fill_ringbuffer(int32_t* buffer, size_t blocksize){
  while(blocksize--){
    audio_t* dst = audio_tx_buffer.getWriteHead();
    size_t ch = USB_AUDIO_CHANNELS;
    while(ch--)
      *dst++ = AUDIO_INT32_TO_SAMPLE(*buffer++); // shift, round, dither, clip, truncate, bitswap
    buffer += AUDIO_CHANNELS - USB_AUDIO_CHANNELS;
    audio_tx_buffer.incrementWriteHead(USB_AUDIO_CHANNELS);
  }
}

void usbd_audio_empty_ringbuffer(uint8_t* buffer, size_t len){
  len /= (AUDIO_BYTES_PER_SAMPLE*USB_AUDIO_CHANNELS);
  audio_t* dst = (audio_t*)buffer;
  size_t available;
  for(size_t i=0; i<len; ++i){
    memcpy(dst, audio_tx_buffer.getReadHead(), USB_AUDIO_CHANNELS*sizeof(audio_t));
    available = audio_tx_buffer.getReadCapacity();
    if(available > USB_AUDIO_CHANNELS)
      audio_tx_buffer.incrementReadHead(USB_AUDIO_CHANNELS);
    else
      adc_underflow++;
    dst += USB_AUDIO_CHANNELS;
  }
}

void usbd_audio_tx_start_callback(uint16_t rate, uint8_t channels){
  // set read head at half a ringbuffer distance from write head
  size_t pos = audio_tx_buffer.getWriteIndex() / USB_AUDIO_CHANNELS;
  size_t len = audio_tx_buffer.getCapacity() / USB_AUDIO_CHANNELS;
  pos = (pos + len/2) % len;
  pos *= USB_AUDIO_CHANNELS;
  audio_tx_buffer.setReadIndex(pos);
#if DEBUG
  printf("start tx %d %d\n", rate, channels);
#endif
}

void usbd_audio_tx_stop_callback(){
#if DEBUG
  printf("stop tx\n");
#endif
}

void usbd_audio_rx_start_callback(uint16_t rate, uint8_t channels){
  audio_rx_buffer.setAll(0);
  size_t pos = audio_rx_buffer.getReadIndex() == 0 ? audio_rx_buffer.getCapacity()/2 : 0;
  audio_rx_buffer.setWriteIndex(pos); // todo: update write index from SAI callbacks and set this correctly
#if DEBUG
  printf("start rx %d %d\n", rate, channels);
#endif
}

void usbd_audio_rx_stop_callback(){
  audio_rx_buffer.setAll(0);
#if DEBUG
  printf("stop rx\n");
#endif
}

void usbd_audio_rx_callback(uint8_t* data, size_t len){
#ifdef USE_USBD_AUDIO_RX
  // copy audio to codec_txbuf aka audio_rx_buffer
  audio_t* src = (audio_t*)data;
  size_t blocksize = len / (USB_AUDIO_CHANNELS*AUDIO_BYTES_PER_SAMPLE);
  while(blocksize--){
    int32_t* dst = audio_rx_buffer.getWriteHead();
    size_t ch = USB_AUDIO_CHANNELS;
    while(ch--)
      *dst++ = AUDIO_SAMPLE_TO_INT32(*src++);
    // should we leave in place or zero out any remaining channels?
    memset(dst, 0, (AUDIO_CHANNELS-USB_AUDIO_CHANNELS)*sizeof(int32_t));
    audio_rx_buffer.incrementWriteHead(AUDIO_CHANNELS);
  }
#endif
}

void usbd_audio_tx_callback(uint8_t* data, size_t len){
#ifdef USE_USBD_AUDIO_TX
  usbd_audio_empty_ringbuffer(data, len);
  usbd_audio_write(data, len);
#endif
}

void usbd_audio_gain_callback(uint8_t gain){
  codec_set_gain_in(gain);
}

void usbd_audio_sync_callback(uint8_t shift){
#ifdef DEBUG
  printf("AUDIO SHIFT %d\n", shift);
#endif
  // todo: do something with the shift number
}
#endif // USE_USB_AUDIO

uint16_t Codec::getBlockSize(){
  return codec_blocksize;
}

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

void Codec::init(){
  codec_txbuf = audio_rx_buffer.getReadHead();
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

#ifdef USE_IIS3DWB
extern "C" {
  void iis3dwb_read();
}

void Codec::start(){
  extern TIM_HandleTypeDef htim8;
  HAL_TIM_Base_Start_IT(&htim8);
  HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_1);
  iis3dwb_read();
}

void Codec::stop(){
  extern TIM_HandleTypeDef htim8;
  HAL_TIM_Base_Stop(&htim8);
}
#endif

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
  extern TIM_HandleTypeDef htim8;
  HAL_TIM_Base_Stop(&htim8);
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
    audio_rx_buffer.setReadIndex(codec_blocksize*AUDIO_CHANNELS);
  }
  void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai){
    audioCallback(codec_rxbuf+codec_blocksize*AUDIO_CHANNELS, codec_txbuf+codec_blocksize*AUDIO_CHANNELS, codec_blocksize);
    audio_rx_buffer.setReadIndex(0);
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
  // start slave first (Noctua)
  ret = HAL_SAI_Transmit_DMA(&hsai_BlockB1, (uint8_t*)codec_txbuf, codec_blocksize*AUDIO_CHANNELS*2);
  ASSERT(ret == HAL_OK, "Failed to start SAI TX DMA");
  ret = HAL_SAI_Receive_DMA(&hsai_BlockA1, (uint8_t*)codec_rxbuf, codec_blocksize*AUDIO_CHANNELS*2);
  ASSERT(ret == HAL_OK, "Failed to start SAI RX DMA");
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

