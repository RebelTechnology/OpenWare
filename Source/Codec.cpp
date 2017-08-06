#include "Codec.h"
#include "errorhandlers.h"
#include "device.h"

#ifdef __cplusplus
 extern "C" {
#endif
   void codec_init(SPI_HandleTypeDef*);
   void codec_write(uint8_t reg, uint8_t data);
   void codec_bypass(int bypass);
   void codec_set_volume(int8_t volume);
#ifdef __cplusplus
}
#endif

#define CODEC_BUFFER_HALFSIZE (CODEC_BUFFER_SIZE/2)
#define CODEC_BUFFER_QUARTSIZE (CODEC_BUFFER_SIZE/4)
static int32_t txbuf[CODEC_BUFFER_SIZE];
static int32_t rxbuf[CODEC_BUFFER_SIZE];

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
  // if(doMute)
  //   codec_set_volume(-1);
  // else
  //   codec_set_volume(128);
  // todo!
  // codec_bypass(doBypass);
}

#ifdef USE_WM8731
extern "C" {
  extern I2S_HandleTypeDef hi2s2;
  extern SPI_HandleTypeDef hspi1;
  extern void audioCallback(int32_t* rx, int32_t* tx, uint16_t size);
}

void Codec::reset(){
  __HAL_I2S_ENABLE(&hi2s2);
  codec_init(&hspi1);
}

void Codec::stop(){
  HAL_I2S_DMAStop(&hi2s2);
}

// hacked in to enable half-complete callbacks
static void I2S_DMARxCplt(DMA_HandleTypeDef *hdma){
  I2S_HandleTypeDef* hi2s = ( I2S_HandleTypeDef* )((DMA_HandleTypeDef* )hdma)->Parent;
  HAL_I2S_RxCpltCallback(hi2s);
}
static void I2S_DMARxHalfCplt(DMA_HandleTypeDef *hdma){
  I2S_HandleTypeDef* hi2s = (I2S_HandleTypeDef*)((DMA_HandleTypeDef*)hdma)->Parent;
  HAL_I2S_RxHalfCpltCallback(hi2s);
}

void Codec::start(){
  HAL_StatusTypeDef ret;
  ret = HAL_I2SEx_TransmitReceive_DMA(&hi2s2, (uint16_t*)txbuf, (uint16_t*)rxbuf, CODEC_BUFFER_SIZE);
  // Ex function doesn't set up a half-complete callback
  extern DMA_HandleTypeDef hdma_spi2_tx;
  extern DMA_HandleTypeDef hdma_i2s2_ext_rx;
  hdma_i2s2_ext_rx.XferHalfCpltCallback = I2S_DMARxHalfCplt;
  hdma_i2s2_ext_rx.XferCpltCallback = I2S_DMARxCplt;
  hdma_spi2_tx.XferHalfCpltCallback = I2S_DMARxHalfCplt;
  hdma_spi2_tx.XferCpltCallback = I2S_DMARxCplt;
  ASSERT(ret == HAL_OK, "Failed to start I2S DMA");
  // while(HAL_I2S_GetState(&hi2s2) != HAL_I2S_STATE_READY); // wait
  // ret = HAL_I2S_Receive_DMA(&hi2s2, (uint16_t*)rxbuf, CODEC_BUFFER_SIZE);
  // ASSERT(ret == HAL_OK, "Failed to start I2S RX DMA");
  // while(HAL_I2S_GetState(&hi2s2) != HAL_I2S_STATE_READY); // wait
  // ret = HAL_I2S_Transmit_DMA(&hi2s2, (uint16_t*)txbuf, CODEC_BUFFER_SIZE);
  // ASSERT(ret == HAL_OK, "Failed to start I2S TX DMA");
}

void Codec::pause(){
  HAL_I2S_DMAPause(&hi2s2);
}

void Codec::resume(){
  HAL_I2S_DMAResume(&hi2s2);
}

extern "C"{
  void HAL_I2SEx_TxRxCpltCallback(I2S_HandleTypeDef *hi2s){
    audioCallback(rxbuf+CODEC_BUFFER_HALFSIZE, txbuf+CODEC_BUFFER_HALFSIZE, CODEC_BUFFER_QUARTSIZE);
  }

  void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s){
    audioCallback(rxbuf, txbuf, CODEC_BUFFER_QUARTSIZE);
  }
  void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s){
    audioCallback(rxbuf+CODEC_BUFFER_HALFSIZE, txbuf+CODEC_BUFFER_HALFSIZE, CODEC_BUFFER_QUARTSIZE);
  }

  void HAL_I2S_ErrorCallback(I2S_HandleTypeDef *hi2s){
    error(CONFIG_ERROR, "SAI DMA Error");
  }
}
#endif /* USE_WM8731 */

#ifdef USE_CS4271
extern "C" {
SAI_HandleTypeDef hsai_BlockA1;
SAI_HandleTypeDef hsai_BlockB1;
SPI_HandleTypeDef hspi4;
}

void Codec::reset(){
  // HAL_SAI_MspInit() is called from HAL_SAI_Init() in MX_SAI1_Init()
  // MX_SAI1_Init();
  // MX_SPI4_Init();

  __HAL_SAI_ENABLE(&hsai_BlockA1);
  __HAL_SAI_ENABLE(&hsai_BlockB1);
  codec_init(&hspi4);

  codec_write(0x01, (1<<3) | (1<<5) | 1); // i2s mode for DAC and ADC
#if defined OWL_TESSERACT || defined OWL_PLAYERF7 || defined OWL_QUADFM
  codec_write(0x06, 0x10 | 0x03 ); // hp filters off
#else
  codec_write(0x06, 0x10); // hp filters on
#endif
  // codec_write(0x01, (1<<3) | (1<<5));
  // codec_write(0x06, (1<<4));
}

void Codec::txrx(){
  HAL_SAI_DMAStop(&hsai_BlockA1);
  HAL_SAI_Transmit_DMA(&hsai_BlockB1, (uint8_t*)rxbuf, CODEC_BUFFER_SIZE);
}

void Codec::stop(){
  HAL_SAI_DMAStop(&hsai_BlockA1);
  HAL_SAI_DMAStop(&hsai_BlockB1);
}

void Codec::start(){
  HAL_StatusTypeDef ret;
  ret = HAL_SAI_Receive_DMA(&hsai_BlockB1, (uint8_t*)rxbuf, CODEC_BUFFER_SIZE);
  ASSERT(ret == HAL_OK, "Failed to start SAI RX DMA");
  ret = HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t*)txbuf, CODEC_BUFFER_SIZE);
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
  audioCallback(rxbuf+CODEC_BUFFER_HALFSIZE, txbuf+CODEC_BUFFER_HALFSIZE, CODEC_BUFFER_QUARTSIZE);
}

void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai){
  audioCallback(rxbuf, txbuf, CODEC_BUFFER_QUARTSIZE);
}

void HAL_SAI_ErrorCallback(SAI_HandleTypeDef *hsai){
  error(CONFIG_ERROR, "SAI DMA Error");
}

}
#endif /* USE_CS4271 */
