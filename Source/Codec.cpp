#include "device.h"
#include "Codec.h"
#include "errorhandlers.h"
#include "ApplicationSettings.h"

static uint16_t blocksize;
static int32_t txbuf[CODEC_BUFFER_SIZE];

uint16_t Codec::getBlockSize(){
  return blocksize;
}

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

void Codec::begin(){
  codec_init();
}

void Codec::reset(){
  stop();
  start();
}

void Codec::ramp(uint32_t max){
  uint32_t incr = max/CODEC_BUFFER_SIZE;
  for(int i=0; i<CODEC_BUFFER_SIZE; ++i)
    txbuf[i] = i*incr;
}

void Codec::clear(){
  set(0);
}

int32_t Codec::getMin(){
  int32_t min = txbuf[0];
  for(int i=1; i<CODEC_BUFFER_SIZE; ++i)
    if(txbuf[i] < min)
      min = txbuf[i];
  return min;
}

int32_t Codec::getMax(){
  int32_t max = txbuf[0];
  for(int i=1; i<CODEC_BUFFER_SIZE; ++i)
    if(txbuf[i] > max)
      max = txbuf[i];
  return max;
}

float Codec::getAvg(){
  float avg = 0;
  for(int i=0; i<CODEC_BUFFER_SIZE; ++i)
    avg += txbuf[i];
  return avg / CODEC_BUFFER_SIZE;
}

void Codec::set(uint32_t value){
  for(int i=0; i<CODEC_BUFFER_SIZE; ++i)
    txbuf[i] = value;
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

#ifdef USE_USB_AUDIO
#include "usbd_audio.h"
#include "RingBuffer.hpp"
#define AUDIO_RINGBUFFER_SIZE 2048
typedef int16_t audio_t;
static audio_t audio_ringbuffer_data[AUDIO_RINGBUFFER_SIZE];
RingBuffer<audio_t> audio_ringbuffer(audio_ringbuffer_data, AUDIO_RINGBUFFER_SIZE);
volatile static size_t adc_underflow = 0;

void fill_buffer(uint8_t* buffer, size_t len){
  len /= (AUDIO_BYTES_PER_SAMPLE*USB_AUDIO_CHANNELS);
  audio_t* dst = (audio_t*)buffer;
  size_t available;
  for(size_t i=0; i<len; ++i){
    memcpy(dst, audio_ringbuffer.getReadHead(), ADS_MAX_CHANNELS*sizeof(audio_t));
    available = audio_ringbuffer.getReadSpace();
    if(available > ADS_MAX_CHANNELS)
      audio_ringbuffer.incrementReadHead(ADS_MAX_CHANNELS);
    else
      adc_underflow++;
    dst += ADS_MAX_CHANNELS;
  }
}

void usbd_audio_gain_callback(uint8_t gain){
  ads_set_gain(gain-24);
}

void usbd_initiate_tx(USBD_HandleTypeDef* pdev, USBD_AUDIO_HandleTypeDef* haudio){
  fill_buffer(haudio->buffer, AUDIO_IN_PACKET_SIZE);
  usbd_audio_write(pdev, haudio->buffer, AUDIO_IN_PACKET_SIZE);
}

void usbd_audio_start_callback(USBD_HandleTypeDef* pdev, USBD_AUDIO_HandleTypeDef* haudio){
 // set read head at half a ringbuffer distance from write head
    size_t pos = audio_ringbuffer.getWritePos() / ADS_MAX_CHANNELS;
    size_t len = audio_ringbuffer.getSize() / ADS_MAX_CHANNELS;
    pos = (pos + len/2) % len;
    pos *= ADS_MAX_CHANNELS;
    audio_ringbuffer.setReadPos(pos);
  usbd_initiate_tx(pdev, haudio);
}

void usbd_audio_data_in_callback(USBD_HandleTypeDef* pdev, USBD_AUDIO_HandleTypeDef* haudio){
  usbd_initiate_tx(pdev, haudio);
}

#endif // USE_USB_AUDIO

void codec_init(){
  blocksize = ADS_BLOCKSIZE;
  ads_setup();
}

void codec_bypass(int bypass){}

void codec_set_gain_in(int8_t volume){
  ads_set_gain(volume);
}

void codec_set_gain_out(int8_t volume){
  ads_set_gain(volume);
}

void Codec::start(){
  ads_start_continuous();
}

void Codec::stop(){
  ads_stop_continuous();
}

extern "C" {
  extern void audioCallback(int32_t* rx, int32_t* tx, uint16_t size);
}

void ads_rx_callback(int32_t* samples, size_t channels, size_t blocksize){
  audioCallback(samples, txbuf, blocksize);
#ifdef USE_USB_AUDIO
  size_t len = channels*blocksize;
  audio_t* dst;
  for(size_t i=0; i<len; i+=ADS_MAX_CHANNELS){
    dst = audio_ringbuffer.getWriteHead(); // assume there's enough contiguous space for one full frame
    // transfer up to 4 channels adc data into ringbuffer
#if ADS_MAX_CHANNELS > 0
    *dst++ = samples[i+ADC_CHANNEL_OFFSET+0]>>ADC_RIGHTSHIFT;
#if ADS_MAX_CHANNELS > 1
    *dst++ = samples[i+ADC_CHANNEL_OFFSET+1]>>ADC_RIGHTSHIFT;
#if ADS_MAX_CHANNELS > 2
    *dst++ = samples[i+ADC_CHANNEL_OFFSET+2]>>ADC_RIGHTSHIFT;
#if ADS_MAX_CHANNELS > 3
    *dst++ = samples[i+ADC_CHANNEL_OFFSET+3]>>ADC_RIGHTSHIFT;
#endif
#endif
#endif
#endif
    audio_ringbuffer.incrementWriteHead(ADS_MAX_CHANNELS);
  }
#endif
}

#endif // USE_ADS1294

#ifdef USE_WM8731
static int32_t rxbuf[CODEC_BUFFER_SIZE];

extern "C" {
  extern I2S_HandleTypeDef hi2s2;
  extern void audioCallback(int32_t* rx, int32_t* tx, uint16_t size);
}

void Codec::stop(){
  HAL_I2S_DMAStop(&hi2s2);
}

void Codec::start(){
  setInputGain(settings.audio_input_gain);
  setOutputGain(settings.audio_output_gain);
  blocksize = min(CODEC_BUFFER_SIZE/4, settings.audio_blocksize);
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
  ret = HAL_I2SEx_TransmitReceive_DMA(&hi2s2, (uint16_t*)txbuf, (uint16_t*)rxbuf, blocksize*4);
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
    audioCallback(rxbuf, txbuf, blocksize);
  }

  void HAL_I2SEx_TxRxCpltCallback(I2S_HandleTypeDef *hi2s){
    audioCallback(rxbuf+blocksize*2, txbuf+blocksize*2, blocksize);
  }

  void HAL_I2S_ErrorCallback(I2S_HandleTypeDef *hi2s){
    error(CONFIG_ERROR, "I2S Error");
  }
  
}
#endif /* USE_WM8731 */

#ifdef USE_CS4271
static int32_t rxbuf[CODEC_BUFFER_SIZE];

extern "C" {
SAI_HandleTypeDef hsai_BlockA1;
SAI_HandleTypeDef hsai_BlockB1;
}

void Codec::txrx(){
  HAL_SAI_DMAStop(&hsai_BlockA1);
  HAL_SAI_Transmit_DMA(&hsai_BlockB1, (uint8_t*)rxbuf, blocksize*4);
}

void Codec::stop(){
  HAL_SAI_DMAStop(&hsai_BlockA1);
  HAL_SAI_DMAStop(&hsai_BlockB1);
}

void Codec::start(){
  setOutputGain(settings.audio_output_gain);
  blocksize = min(CODEC_BUFFER_SIZE/4, settings.audio_blocksize);
  HAL_StatusTypeDef ret;
  ret = HAL_SAI_Receive_DMA(&hsai_BlockB1, (uint8_t*)rxbuf, blocksize*4);
  ASSERT(ret == HAL_OK, "Failed to start SAI RX DMA");
  ret = HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t*)txbuf, blocksize*4);
  ASSERT(ret == HAL_OK, "Failed to start SAI TX DMA");
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

extern void audioCallback(int32_t* rx, int32_t* tx, uint16_t size);

void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai){
  audioCallback(rxbuf+blocksize*2, txbuf+blocksize*2, blocksize);
}

void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai){
  audioCallback(rxbuf, txbuf, blocksize);
}

void HAL_SAI_ErrorCallback(SAI_HandleTypeDef *hsai){
  error(CONFIG_ERROR, "SAI DMA Error");
}

}
#endif /* USE_CS4271 */

