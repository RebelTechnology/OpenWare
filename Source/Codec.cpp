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

#ifdef USE_USB_AUDIO
#include "usbd_audio.h"
#include "SerialBuffer.hpp"
SerialBuffer<AUDIO_RINGBUFFER_SIZE, audio_t> audio_ringbuffer;
volatile static size_t adc_underflow = 0;

void usbd_audio_fill_ringbuffer(int32_t* buffer, size_t blocksize){
  while(blocksize--){
    audio_t* dst = audio_ringbuffer.getWriteHead();
    size_t ch = USB_AUDIO_CHANNELS;
    while(ch--)
      *dst++ = *buffer++; // todo: shift, round, dither, 
    buffer += AUDIO_CHANNELS - USB_AUDIO_CHANNELS;
    audio_ringbuffer.incrementWriteHead(USB_AUDIO_CHANNELS);
  }
}

void usbd_audio_empty_ringbuffer(uint8_t* buffer, size_t len){
  len /= (AUDIO_BYTES_PER_SAMPLE*USB_AUDIO_CHANNELS);
  audio_t* dst = (audio_t*)buffer;
  size_t available;
  for(size_t i=0; i<len; ++i){
    memcpy(dst, audio_ringbuffer.getReadHead(), USB_AUDIO_CHANNELS*sizeof(audio_t));
    available = audio_ringbuffer.getReadCapacity();
    if(available > USB_AUDIO_CHANNELS)
      audio_ringbuffer.incrementReadHead(USB_AUDIO_CHANNELS);
    else
      adc_underflow++;
    dst += USB_AUDIO_CHANNELS;
  }
}

void usbd_initiate_tx(USBD_HandleTypeDef* pdev, USBD_AUDIO_HandleTypeDef* haudio){
  usbd_audio_empty_ringbuffer(haudio->audio_in_buffer, AUDIO_IN_PACKET_SIZE);
  usbd_audio_write(pdev, haudio->audio_in_buffer, AUDIO_IN_PACKET_SIZE);
}

void usbd_audio_start_callback(USBD_HandleTypeDef* pdev, USBD_AUDIO_HandleTypeDef* haudio){
  // set read head at half a ringbuffer distance from write head
  size_t pos = audio_ringbuffer.getWriteIndex() / USB_AUDIO_CHANNELS;
  size_t len = audio_ringbuffer.getCapacity() / USB_AUDIO_CHANNELS;
  pos = (pos + len/2) % len;
  pos *= USB_AUDIO_CHANNELS;
  audio_ringbuffer.setReadIndex(pos);
  // usbd_initiate_tx(pdev, haudio);
  // haudio->tx_audio_active = 1;
}

void usbd_audio_data_out_callback(USBD_HandleTypeDef* pdev, USBD_AUDIO_HandleTypeDef* haudio){
}

void usbd_audio_data_in_callback(USBD_HandleTypeDef* pdev, USBD_AUDIO_HandleTypeDef* haudio){
  usbd_initiate_tx(pdev, haudio);
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

#ifdef USE_IIS3DWB

#include "iis3dwb_reg.h"

extern "C" {
  stmdev_ctx_t dev_ctx;
  uint8_t dev_xl_data[6];
  extern SPI_HandleTypeDef hspi4;

static int32_t platform_write(void *handle, uint8_t reg, uint8_t *bufp,
                              uint16_t len);
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len);

#define CS_SPI4_GPIO_Port   GPIOD      // PD5
#define CS_SPI4_Pin         GPIO_PIN_5 // PD5

/*
 * @brief  Write generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to write
 * @param  bufp      pointer to data to write in register reg
 * @param  len       number of consecutive register to write
 *
 */
static int32_t platform_write(void *handle, uint8_t reg, uint8_t *bufp,
                              uint16_t len) {
  if (handle == &hspi4) {
    /* Write multiple command */
    reg |= 0x40;
    HAL_GPIO_WritePin(CS_SPI4_GPIO_Port, CS_SPI4_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit((SPI_HandleTypeDef*)handle, &reg, 1, 1000);
    HAL_SPI_Transmit((SPI_HandleTypeDef*)handle, bufp, len, 1000);
    HAL_GPIO_WritePin(CS_SPI4_GPIO_Port, CS_SPI4_Pin, GPIO_PIN_SET);
  }
  return 0;
}

/*
 * @brief  Read generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to read
 * @param  bufp      pointer to buffer that store the data read
 * @param  len       number of consecutive register to read
 *
 */
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len)
{
  if (handle == &hspi4){
    /* Read multiple command */
    reg |= 0xC0;
    HAL_GPIO_WritePin(CS_SPI4_GPIO_Port, CS_SPI4_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit((SPI_HandleTypeDef*)handle, &reg, 1, 1000);
    HAL_SPI_Receive((SPI_HandleTypeDef*)handle, bufp, len, 1000);
    HAL_GPIO_WritePin(CS_SPI4_GPIO_Port, CS_SPI4_Pin, GPIO_PIN_SET);
  }
  return 0;
}

  void codec_init(){
    dev_ctx.write_reg = platform_write;
    dev_ctx.read_reg = platform_read;
    dev_ctx.handle = &hspi4;
    iis3dwb_xl_full_scale_set(&dev_ctx, IIS3DWB_2g);
    iis3dwb_xl_data_rate_set(&dev_ctx, IIS3DWB_XL_ODR_26k7Hz);
    iis3dwb_xl_axis_selection_set(&dev_ctx, IIS3DWB_ENABLE_ALL);
  }

  void codec_bypass(int bypass){}
  void codec_set_gain_in(int8_t volume){}
  void codec_set_gain_out(int8_t volume){}
}

typedef union{
  int16_t i16bit[3];
  uint8_t u8bit[6];
} axis3bit16_t;

axis3bit16_t dev_data_raw_acceleration;
iis3dwb_status_reg_t dev_status;

void iis3dwb_read(){
  iis3dwb_status_reg_get(&dev_ctx, &dev_status);
  // if(dev_status.xlda)
    iis3dwb_acceleration_raw_get(&dev_ctx, dev_data_raw_acceleration.u8bit);
}

static size_t rxindex = 0;
static size_t rxhalf = 0;
static size_t rxfull = 0;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
  extern TIM_HandleTypeDef htim8;
  if (htim == &htim8){
    // sample all channels
    iis3dwb_read();
#if defined USE_USB_AUDIO && defined AUDIO_BYPASS
    // write directly to usb buffer
    audio_t* dst = audio_ringbuffer.getWriteHead(); // assume there's enough contiguous space for one full frame
#else
    audio_t* dst = codec_rxbuf + rxindex;
#endif
    *dst++ = dev_data_raw_acceleration.i16bit[0];
    *dst++ = dev_data_raw_acceleration.i16bit[1];
    *dst++ = dev_data_raw_acceleration.i16bit[2];
    // memcpy(dst, ads_samples, AUDIO_CHANNELS*sizeof(audio_t));
    // dst += ADS_ACTIVE_CHANNELS;
#if defined USE_USB_AUDIO && defined AUDIO_BYPASS
    audio_ringbuffer.incrementWriteHead(AUDIO_CHANNELS);
#else
    rxindex += AUDIO_CHANNELS;
    if(rxindex == rxhalf){
      audioCallback(codec_rxbuf, codec_txbuf, codec_blocksize); // trigger audio processing block
    }else if(rxindex >= rxfull){
      rxindex = 0;
      audioCallback(codec_rxbuf+rxhalf, codec_txbuf+rxhalf, codec_blocksize);
    }
#endif
  }
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
  ret = HAL_SAI_Receive_DMA(&hsai_BlockA1, (uint8_t*)codec_rxbuf, codec_blocksize*AUDIO_CHANNELS*2);
  ASSERT(ret == HAL_OK, "Failed to start SAI RX DMA");
  ret = HAL_SAI_Transmit_DMA(&hsai_BlockB1, (uint8_t*)codec_txbuf, codec_blocksize*AUDIO_CHANNELS*2);
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

}

#endif /* USE_PCM3168A */

