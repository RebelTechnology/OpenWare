#include "device.h"
#include "Codec.h"
#include "errorhandlers.h"
#include "ApplicationSettings.h"

extern "C" {
  extern void audioCallback(int32_t* rx, int32_t* tx, uint16_t size);
}

static uint16_t blocksize = 0;
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

#ifdef USE_PCM3168A
#include "pcm3168a.h"

void Codec::start(){
}

void Codec::stop(){
}


#endif /* USE_PCM3168A */


#ifdef USE_ADS1294
#include "ads.h"

static int32_t rxbuf[CODEC_BUFFER_SIZE];
static size_t rxindex = 0;
static size_t rxhalf = 0;
static size_t rxfull = 0;

#ifdef USE_USB_AUDIO
#include "usbd_audio.h"
#include "RingBuffer.hpp"
typedef int32_t audio_t;
static audio_t audio_ringbuffer_data[AUDIO_RINGBUFFER_SIZE] CCM;
RingBuffer<audio_t> audio_ringbuffer(audio_ringbuffer_data, AUDIO_RINGBUFFER_SIZE);
volatile static size_t adc_underflow = 0;

static int32_t ads_samples[ADS_MAX_CHANNELS] CCM;

#ifdef USE_KX122
#include "kx122.h"
static int32_t kx122_samples[KX122_TOTAL_CHANNELS] CCM;
#endif

void fill_buffer(uint8_t* buffer, size_t len){
  len /= (AUDIO_BYTES_PER_SAMPLE*USB_AUDIO_CHANNELS);
  audio_t* dst = (audio_t*)buffer;
  size_t available;
  for(size_t i=0; i<len; ++i){
    memcpy(dst, audio_ringbuffer.getReadHead(), USB_AUDIO_CHANNELS*sizeof(audio_t));
    available = audio_ringbuffer.getReadSpace();
    if(available > USB_AUDIO_CHANNELS)
      audio_ringbuffer.incrementReadHead(USB_AUDIO_CHANNELS);
    else
      adc_underflow++;
    dst += USB_AUDIO_CHANNELS;
  }
}

void usbd_audio_gain_callback(uint8_t gain){
  ads_set_gain(gain-24);
}

void usbd_initiate_tx(USBD_HandleTypeDef* pdev, USBD_AUDIO_HandleTypeDef* haudio){
  fill_buffer(haudio->audio_out_buffer, AUDIO_IN_PACKET_SIZE);
  usbd_audio_write(pdev, haudio->audio_out_buffer, AUDIO_IN_PACKET_SIZE);
}

void usbd_audio_start_callback(USBD_HandleTypeDef* pdev, USBD_AUDIO_HandleTypeDef* haudio){
  // set read head at half a ringbuffer distance from write head
  size_t pos = audio_ringbuffer.getWritePos() / USB_AUDIO_CHANNELS;
  size_t len = audio_ringbuffer.getSize() / USB_AUDIO_CHANNELS;
  pos = (pos + len/2) % len;
  pos *= USB_AUDIO_CHANNELS;
  audio_ringbuffer.setReadPos(pos);
  usbd_initiate_tx(pdev, haudio);
}

void usbd_audio_data_in_callback(USBD_HandleTypeDef* pdev, USBD_AUDIO_HandleTypeDef* haudio){
  usbd_initiate_tx(pdev, haudio);
}

#endif // USE_USB_AUDIO

void codec_init(){
  blocksize = AUDIO_BLOCK_SIZE;
  rxindex = 0;
  rxhalf = blocksize*USB_AUDIO_CHANNELS;
  rxfull = 2*rxhalf;

  extern TIM_HandleTypeDef htim8;
  HAL_TIM_Base_Start_IT(&htim8);
  HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_4);
  
  ads_setup(ads_samples);
#ifdef USE_KX122
  kx122_setup(kx122_samples);
#endif
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

extern "C"{
  void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
    extern TIM_HandleTypeDef htim8;
    if (htim == &htim8){
      // sample all channels

#ifdef AUDIO_BYPASS
      // write directly to usb buffer
      audio_t* dst = audio_ringbuffer.getWriteHead(); // assume there's enough contiguous space for one full frame
#else
      audio_t* dst = rxbuf + rxindex;
#endif
      memcpy(dst, ads_samples, ADS_ACTIVE_CHANNELS*sizeof(audio_t));
      dst += ADS_ACTIVE_CHANNELS;
#ifdef USE_KX122
      memcpy(dst, kx122_samples, KX122_ACTIVE_CHANNELS*sizeof(audio_t));
      dst += KX122_ACTIVE_CHANNELS;
#endif
#ifdef AUDIO_BYPASS
      audio_ringbuffer.incrementWriteHead(USB_AUDIO_CHANNELS);
#else
      rxindex += USB_AUDIO_CHANNELS;
      if(rxindex == rxhalf){
	audioCallback(rxbuf, txbuf, blocksize); // trigger audio processing block
	audio_ringbuffer.write(txbuf+rxhalf, blocksize*USB_AUDIO_CHANNELS); // copy back previous block
      }else if(rxindex >= rxfull){
	rxindex = 0;
	audioCallback(rxbuf+rxhalf, txbuf+rxhalf, blocksize);
	audio_ringbuffer.write(txbuf, blocksize*USB_AUDIO_CHANNELS);
      }
#endif
    // size_t available = audio_ringbuffer.getReadSpace();
    // if(available > AUDIO_RINGBUFFER_OVER_LIMIT){
    //   // adjust timer period
    // }else if(available < AUDIO_RINGBUFFER_UNDER_LIMIT){
    //   // adjust timer period
    // }
    }
  }
}

extern "C" {
  // void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi){
  // }
  extern SPI_HandleTypeDef ADS_HSPI;
#ifdef USE_KX122
  extern SPI_HandleTypeDef KX122_HSPI;
#endif
  void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi){
    if(hspi == &ADS_HSPI)
      ads_cplt();
#ifdef USE_KX122
    else if(hspi == &KX122_HSPI)
      kx122_cplt();
#endif
  }
  void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi){
    if(hspi == &ADS_HSPI)
      ads_cplt();
#ifdef USE_KX122
    else if(hspi == &KX122_HSPI)
      kx122_cplt();
#endif
  }
  void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi){
    if(hspi == &ADS_HSPI)
      error(RUNTIME_ERROR, "ADS SPI Error");
#ifdef USE_KX122
    else if(hspi == &KX122_HSPI)
      error(RUNTIME_ERROR, "KX122 SPI Error");
#endif
    else
      error(RUNTIME_ERROR, "SPI Error");
  }

};

#endif // USE_ADS1294

#ifdef USE_WM8731
static int32_t rxbuf[CODEC_BUFFER_SIZE];

extern "C" {
  extern I2S_HandleTypeDef hi2s2;
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

