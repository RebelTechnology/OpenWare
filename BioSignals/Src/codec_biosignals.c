#include "Codec.h"
#include "device.h"
#include "errorhandlers.h"
#include "ads.h"

static size_t rxindex = 0;
static size_t rxhalf = 0;
static size_t rxfull = 0;
extern uint16_t codec_blocksize;
extern int32_t* codec_rxbuf;
extern int32_t* codec_txbuf;


typedef int32_t audio_t;
static audio_t ads_samples[ADS_MAX_CHANNELS];
#ifdef USE_KX122
#include "kx122.h"
static audio_t kx122_samples[KX122_TOTAL_CHANNELS];
#endif


#ifdef USE_USBD_AUDIO
#include "usbd_audio.h"
#include "SerialBuffer.hpp"
SerialBuffer<AUDIO_RINGBUFFER_SIZE, audio_t> audio_ringbuffer;
volatile static size_t adc_underflow = 0;

void usbd_fill_buffer(uint8_t* buffer, size_t len){
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
  usbd_fill_buffer(haudio->audio_out_buffer, AUDIO_IN_PACKET_SIZE);
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

#endif // USE_USBD_AUDIO

void codec_init(){
  rxindex = 0;
  rxhalf = AUDIO_BLOCK_SIZE*AUDIO_CHANNELS;
  rxfull = 2*rxhalf;  
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

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
  extern TIM_HandleTypeDef htim8;
  if (htim == &htim8){
    // sample all channels

#if defined USE_USBD_AUDIO && defined AUDIO_BYPASS
    // write directly to usb buffer
    audio_t* dst = audio_ringbuffer.getWriteHead(); // assume there's enough contiguous space for one full frame
#else
    audio_t* dst = codec_rxbuf + rxindex;
#endif
    memcpy(dst, ads_samples, ADS_ACTIVE_CHANNELS*sizeof(audio_t));
    dst += ADS_ACTIVE_CHANNELS;
#ifdef USE_KX122
    memcpy(dst, kx122_samples, KX122_ACTIVE_CHANNELS*sizeof(audio_t));
    dst += KX122_ACTIVE_CHANNELS;
#endif
#if defined USE_USBD_AUDIO && defined AUDIO_BYPASS
    audio_ringbuffer.incrementWriteHead(USB_AUDIO_CHANNELS);
#else
    rxindex += USB_AUDIO_CHANNELS;
    if(rxindex == rxhalf){
      audioCallback(codec_rxbuf, codec_txbuf, codec_blocksize); // trigger audio processing block
#ifdef USE_USBD_AUDIO
      audio_ringbuffer.write(codec_txbuf+rxhalf, codec_blocksize*USB_AUDIO_CHANNELS); // copy back previous block
#endif
    }else if(rxindex >= rxfull){
      rxindex = 0;
      audioCallback(codec_rxbuf+rxhalf, codec_txbuf+rxhalf, codec_blocksize);
#ifdef USE_USBD_AUDIO
      audio_ringbuffer.write(codec_txbuf, codec_blocksize*USB_AUDIO_CHANNELS);
#endif
    }
#endif
  }
}

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
