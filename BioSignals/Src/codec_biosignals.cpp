#include <cstddef>
#include <cstring>
#include "Codec.h"
#include "device.h"
#include "errorhandlers.h"
#include "ads.h"
#include "ads1298.h"

static size_t rxindex = 0;
static size_t rxhalf = 0;
static size_t rxfull = 0;

typedef int32_t audio_t;
static audio_t ads_samples[ADS_MAX_CHANNELS];
#ifdef USE_KX122
#include "kx122.h"
static audio_t kx122_samples[KX122_TOTAL_CHANNELS];
#endif

extern "C" {
  extern uint16_t codec_blocksize;
  extern int32_t codec_rxbuf[CODEC_BUFFER_SIZE] DMA_RAM;
  extern int32_t codec_txbuf[CODEC_BUFFER_SIZE] DMA_RAM;
}

void codec_init(){
  rxindex = 0;
  rxhalf = codec_blocksize*AUDIO_CHANNELS;
  rxfull = 2*rxhalf;  
  ads_setup(ads_samples);
#ifdef USE_KX122
  kx122_setup(kx122_samples);
#endif
}

void codec_reset(){
  rxhalf = codec_blocksize*AUDIO_CHANNELS;
  rxfull = 2*rxhalf;    
}

void codec_bypass(int bypass){}

void codec_set_gain_in(int8_t volume){
}

void codec_set_gain_out(int8_t volume){
  volume /= 10;
  if(volume > 8)
    ads_set_gain(ADS1298::GAIN_12X);
  else if(volume > 4)
    ads_set_gain(ADS1298::GAIN_8X);
  else if(volume > 2)
    ads_set_gain(ADS1298::GAIN_4X);
  else if(volume > 1)
    ads_set_gain(ADS1298::GAIN_2X);
  else
    ads_set_gain(ADS1298::GAIN_1X);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
  extern TIM_HandleTypeDef htim8;
  if (htim == &htim8){
    // sample all channels
    audio_t* dst = codec_rxbuf + rxindex;
    memcpy(dst, ads_samples, ADS_ACTIVE_CHANNELS*sizeof(audio_t));
    dst += ADS_ACTIVE_CHANNELS;
#ifdef USE_KX122
    memcpy(dst, kx122_samples, KX122_ACTIVE_CHANNELS*sizeof(audio_t));
    dst += KX122_ACTIVE_CHANNELS;
#endif
    rxindex += AUDIO_CHANNELS;
    if(rxindex == rxhalf){
      audioCallback(codec_rxbuf, codec_txbuf, codec_blocksize); // trigger audio processing block
    }else if(rxindex >= rxfull){
      rxindex = 0;
      audioCallback(codec_rxbuf+rxhalf, codec_txbuf+rxhalf, codec_blocksize);
    }
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
