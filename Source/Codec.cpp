#include "device.h"
#include "Owl.h"
#include "Codec.h"
#include "errorhandlers.h"
#include "ApplicationSettings.h"
#include <cstring>
#include "ProgramManager.h"

#include "CircularBuffer.h"

#ifdef USE_CS4271
#define HSAI_RX hsai_BlockB1
#define HSAI_TX hsai_BlockA1
#define HDMA_RX hdma_sai1_b
#define HDMA_TX hdma_sai1_a
#else
#define HSAI_RX hsai_BlockA1
#define HSAI_TX hsai_BlockB1
#define HDMA_RX hdma_sai1_a
#define HDMA_TX hdma_sai1_b
#endif

extern "C" {
  uint16_t codec_blocksize = 0;
  int32_t codec_rxbuf[CODEC_BUFFER_SIZE] DMA_RAM;
  int32_t codec_txbuf[CODEC_BUFFER_SIZE] DMA_RAM;
  extern DMA_HandleTypeDef HDMA_TX;
  extern DMA_HandleTypeDef HDMA_RX;
#if defined USE_CS4271 || defined USE_PCM3168A
  extern SAI_HandleTypeDef HSAI_RX;
  extern SAI_HandleTypeDef HSAI_TX;
#endif
}

#ifdef USE_USBD_AUDIO
#include "usbd_audio.h"

CircularBuffer<audio_t>* volatile usbd_rx = NULL;
CircularBuffer<audio_t>* volatile usbd_tx = NULL;

// static void update_rx_read_index(){
// #if defined USE_CS4271 || defined USE_PCM3168A
//   // NDTR: the number of remaining data units in the current DMA Stream transfer.
//   // use HDMA_TX position in case we have stopped HDMA_RX
//   // todo: if(wet) then read position is incremented by audioCallback ?
//   size_t pos = CODEC_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(&HDMA_TX);
//   // // mask to full frame (assumes AUDIO_CHANNELS is a power of two)
//   // audio_rx_buffer.setReadIndex(pos & ~(AUDIO_CHANNELS-1));
//   audio_rx_buffer.setReadIndex(pos);
// #endif
// }

// static void update_tx_write_index(){
// #if defined USE_CS4271 || defined USE_PCM3168A
//   // NDTR: the number of remaining data units in the current DMA Stream transfer.
//   size_t pos = CODEC_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(&HDMA_TX);
//   // // mask to full frame (assumes AUDIO_CHANNELS is a power of two)
//   // audio_tx_buffer.setWriteIndex(pos & ~(AUDIO_CHANNELS-1));
//   audio_tx_buffer.setWriteIndex(pos);
// #endif
// }

void usbd_audio_tx_start_callback(size_t rate, uint8_t channels, void* cb){
  usbd_tx = (CircularBuffer<audio_t>*)cb;
  usbd_tx->reset();
  usbd_tx->clear();
  usbd_tx->moveWriteHead(usbd_tx->getSize()/2);
#ifdef DEBUG
  printf("start tx %u %u %u\n", rate, channels, usbd_tx->getSize());
#endif
#if 0 && defined USE_USBD_AUDIO_TX && USBD_AUDIO_TX_CHANNELS > 0
  update_tx_write_index();
  size_t pos = audio_tx_buffer.getWriteIndex(); // not always aligned to AUDIO_CHANNELS samples
  // mask to full frame (assumes AUDIO_CHANNELS is a power of two)
  pos &= ~(AUDIO_CHANNELS-1);
  // position read head at one USB transfer block back from write head
  pos -= AUDIO_CHANNELS*rate/1000;
  // // position read head at half a ringbuffer distance from write head
  // pos -= audio_tx_buffer.getSize()/2;
  audio_tx_buffer.setReadIndex(pos);

  // // set read head at half a ringbuffer distance from write head
  // update_tx_write_index();
  // size_t pos = audio_tx_buffer.getWriteIndex();
  // size_t len = audio_tx_buffer.getSize();
  // pos = (pos + len/2) % len;
  // pos = (pos/AUDIO_CHANNELS)*AUDIO_CHANNELS; // round down to nearest frame
  // audio_tx_buffer.setReadIndex(pos);
#ifdef DEBUG
  printf("start tx %u %u %u\n", rate, channels, pos);
#endif
#endif
}

static int32_t usbd_audio_tx_flow = 0;
static uint32_t usbd_audio_rx_count = 0;
// expect a 1 in 10k sample underflow (-0.01% sample accuracy)
void usbd_audio_tx_callback(uint8_t* data, size_t len){
#if 0 && defined USE_USBD_AUDIO_TX && USBD_AUDIO_TX_CHANNELS > 0
  update_tx_write_index();
  size_t available = audio_tx_buffer.getReadCapacity()/AUDIO_CHANNELS;
  size_t blocksize = len / (USBD_AUDIO_TX_CHANNELS*AUDIO_BYTES_PER_SAMPLE);
  if(available < blocksize){
    usbd_audio_tx_flow += blocksize-available;
    blocksize = available;
    len = blocksize*USBD_AUDIO_TX_CHANNELS*AUDIO_BYTES_PER_SAMPLE;
  }
  audio_t* dst = (audio_t*)data;
  while(blocksize--){
    int32_t* src = audio_tx_buffer.getReadHead();
    size_t ch = USBD_AUDIO_TX_CHANNELS;
    while(ch--)
      *dst++ = AUDIO_INT32_TO_SAMPLE(*src++); // shift, round, dither, clip, truncate, bitswap
    audio_tx_buffer.moveReadHead(AUDIO_CHANNELS);
  }
  usbd_audio_write(data, len);
#endif
}

void usbd_audio_tx_stop_callback(){
  usbd_tx = NULL;
#ifdef DEBUG
  printf("stop tx\n");
#endif
}

void usbd_audio_rx_start_callback(size_t rate, uint8_t channels, void* cb){
  usbd_rx = (CircularBuffer<audio_t>*)cb;
  usbd_rx->reset();
  usbd_rx->clear();
  usbd_rx->moveReadHead(usbd_rx->getSize()/2);
  usbd_audio_rx_count = 0;
  HAL_SAI_DMAPause(&HSAI_RX);
#ifdef DEBUG
  printf("start rx %u %u %u\n", rate, channels, usbd_rx->getSize());
#endif
#if 0 && defined USE_USBD_AUDIO_RX && USBD_AUDIO_RX_CHANNELS > 0
  // todo: if(wet) { disable RX dma } else { stop patch }
  // __HAL_DMA_DISABLE(&HDMA_RX); // stop codec transfers
  HAL_SAI_DMAPause(&HSAI_RX);
  audio_rx_buffer.clear();
  update_rx_read_index();
  size_t pos = audio_rx_buffer.getWriteIndex();
  size_t len = audio_rx_buffer.getSize();
  pos = (pos + len/2) % len;
  pos = (pos/AUDIO_CHANNELS)*AUDIO_CHANNELS; // round down to nearest frame
  audio_rx_buffer.setWriteIndex(pos);
  // program.exitProgram(true);
  // owl.setOperationMode(STREAM_MODE);
#ifdef DEBUG
  printf("start rx %u %u %u\n", rate, channels, pos);
#endif
#endif
}

void usbd_audio_rx_stop_callback(){
  usbd_rx = NULL;
  HAL_SAI_DMAResume(&HSAI_RX);
#if 0 && defined USE_USBD_AUDIO_RX && USBD_AUDIO_RX_CHANNELS > 0
  // __HAL_DMA_ENABLE(&HDMA_RX); // restart codec transfers
  HAL_SAI_DMAResume(&HSAI_RX);
  audio_rx_buffer.clear();
  // program.loadProgram(program.getProgramIndex());
  // program.startProgram(true);
  // owl.setOperationMode(RUN_MODE);
#endif  
#ifdef DEBUG
  printf("stop rx\n");
#endif
}

static int32_t usbd_audio_rx_flow = 0;
size_t usbd_audio_rx_callback(uint8_t* data, size_t len){
#if 0 && defined USE_USBD_AUDIO_RX && USBD_AUDIO_RX_CHANNELS > 0
  // copy audio to codec_txbuf aka audio_rx_buffer
  update_rx_read_index();
  audio_t* src = (audio_t*)data;
  size_t blocksize = len / (USBD_AUDIO_RX_CHANNELS*AUDIO_BYTES_PER_SAMPLE);
  size_t available = audio_rx_buffer.getWriteCapacity()/AUDIO_CHANNELS;
  if(available < blocksize){
    usbd_audio_rx_flow += blocksize-available;
    // skip some frames start and end of this block
    // src += (blocksize - available)*USBD_AUDIO_RX_CHANNELS/2;
    blocksize = available;
    len = blocksize*USBD_AUDIO_RX_CHANNELS*AUDIO_BYTES_PER_SAMPLE;
  }
  while(blocksize--){
      int32_t* dst = audio_rx_buffer.getWriteHead();
      size_t ch = USBD_AUDIO_RX_CHANNELS;
      while(ch--)
  	*dst++ = AUDIO_SAMPLE_TO_INT32(*src++);
      // should we leave in place or zero out any remaining channels?
      memset(dst, 0, (AUDIO_CHANNELS-USBD_AUDIO_RX_CHANNELS)*sizeof(int32_t));
      audio_rx_buffer.moveWriteHead(AUDIO_CHANNELS);
  }
  // available = audio_rx_buffer.getWriteCapacity()*AUDIO_BYTES_PER_SAMPLE*USBD_AUDIO_RX_CHANNELS/AUDIO_CHANNELS;
  // if(available < AUDIO_RX_PACKET_SIZE)
  //   return available;
  usbd_audio_rx_count += len;
#endif
  return len;
}

void usbd_audio_mute_callback(int16_t gain){
  // todo!
}

void usbd_audio_gain_callback(int16_t gain){
  // codec_set_gain_in(gain); todo!
}

/* Get number of samples transmitted since previous request */
uint32_t usbd_audio_get_rx_count(){
  return 0;
  // // NDTR: the number of remaining data units in the current DMA Stream transfer.
  // size_t pos = CODEC_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(&HDMA_TX);
  // pos += usbd_audio_rx_count;
  // usbd_audio_rx_count = 0;
  // return pos / AUDIO_CHANNELS;
}
#endif // USE_USBD_AUDIO

uint16_t Codec::getBlockSize(){
  return codec_blocksize;
}

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

void Codec::init(){
  // todo: if(wet):
  // audio_tx_buffer = CircularBuffer<int32_t>(codec_txbuf, CODEC_BUFFER_SIZE);
  // audio_rx_buffer = CircularBuffer<int32_t>(codec_rxbuf, CODEC_BUFFER_SIZE);
  // todo: else
  // audio_tx_buffer = CircularBuffer<int32_t>(codec_rxbuf, CODEC_BUFFER_SIZE);
  // audio_rx_buffer = CircularBuffer<int32_t>(codec_txbuf, CODEC_BUFFER_SIZE);
  // audio_tx_buffer.reset();
  // audio_rx_buffer.reset();
  codec_init();
}

void Codec::reset(){
  // todo: this is called when blocksize is changed
  stop();
  // audio_tx_buffer.reset();
  // audio_rx_buffer.reset();
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
  for(int i=0; i<CODEC_BUFFER_SIZE; ++i){
    codec_txbuf[i] = value;
    codec_rxbuf[i] = value;
  }
}

void Codec::bypass(bool doBypass){
  codec_bypass(doBypass);
}

void Codec::mute(bool doMute){
  codec_set_gain_out(0); // todo: fixme!
}

void Codec::setInputGain(int8_t value){
  codec_set_gain_in(value);
}

void Codec::setOutputGain(int8_t value){
  codec_set_gain_out(value);
}

void Codec::setHighPass(bool hpf){
#ifdef USE_PCM3168A
  if(hpf)
    codec_write(82, 0b00000000); // enable HPF for all ADC channels
  else
    codec_write(82, 0b00000111); // disable HPF for all ADC channels
#endif
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
  // codec_blocksize = min(AUDIO_BLOCK_SIZE, settings.audio_blocksize);
  codec_blocksize = AUDIO_BLOCK_SIZE;
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
  ret = HAL_I2SEx_TransmitReceive_DMA(&hi2s2, (uint16_t*)codec_txbuf, (uint16_t*)codec_rxbuf, codec_blocksize*AUDIO_CHANNELS*2);
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
  void usbd_rx_convert(int32_t* dst, size_t len){
    usbd_audio_rx_count += len;
    while(len--)
      *dst++ = AUDIO_SAMPLE_TO_INT32(usbd_rx->read());
  }
  void usbd_tx_convert(int32_t* src, size_t len){
#if USBD_AUDIO_TX_CHANNELS != AUDIO_CHANNELS
#error "todo: support for USBD_AUDIO_TX_CHANNELS != AUDIO_CHANNELS"
#endif
    while(len--)
      // macro handles shift, round, dither, clip, truncate, bitswap
      usbd_tx->write(AUDIO_INT32_TO_SAMPLE(*src++));
  }
  void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai){
    if(usbd_tx)
      usbd_tx_convert(codec_txbuf, codec_blocksize*AUDIO_CHANNELS);
    if(usbd_rx)
      usbd_rx_convert(codec_rxbuf, codec_blocksize*AUDIO_CHANNELS);
    audioCallback(codec_rxbuf, codec_txbuf, codec_blocksize);
  }
  void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai){
    if(usbd_tx)
      usbd_tx_convert(codec_txbuf+codec_blocksize*AUDIO_CHANNELS, codec_blocksize*AUDIO_CHANNELS);
    if(usbd_rx)
      usbd_rx_convert(codec_rxbuf+codec_blocksize*AUDIO_CHANNELS, codec_blocksize*AUDIO_CHANNELS);
    audioCallback(codec_rxbuf+codec_blocksize*AUDIO_CHANNELS, codec_txbuf+codec_blocksize*AUDIO_CHANNELS, codec_blocksize);
  }
  void HAL_SAI_ErrorCallback(SAI_HandleTypeDef *hsai){
    error(CONFIG_ERROR, "SAI DMA Error");
  }
}

void Codec::txrx(){
  HAL_SAI_DMAStop(&HSAI_RX);
  HAL_SAI_Transmit_DMA(&HSAI_RX, (uint8_t*)codec_rxbuf, codec_blocksize*AUDIO_CHANNELS*2);
}

void Codec::stop(){
  HAL_SAI_DMAStop(&HSAI_RX);
  HAL_SAI_DMAStop(&HSAI_TX);
}

void Codec::start(){
  setOutputGain(settings.audio_output_gain);
  // codec_blocksize = min(CODEC_BUFFER_SIZE/(AUDIO_CHANNELS*2), settings.audio_blocksize);
  codec_blocksize = CODEC_BUFFER_SIZE/(AUDIO_CHANNELS*2);
  HAL_StatusTypeDef ret;
#ifdef USE_CS4271
  ret = HAL_SAI_Receive_DMA(&HSAI_RX, (uint8_t*)codec_rxbuf, codec_blocksize*AUDIO_CHANNELS*2);
  if(ret == HAL_OK)
    ret = HAL_SAI_Transmit_DMA(&HSAI_TX, (uint8_t*)codec_txbuf, codec_blocksize*AUDIO_CHANNELS*2);
#else // PCM3168A
  // start slave first (Noctua)
  ret = HAL_SAI_Transmit_DMA(&HSAI_TX, (uint8_t*)codec_txbuf, codec_blocksize*AUDIO_CHANNELS*2);
  if(ret == HAL_OK)
    ret = HAL_SAI_Receive_DMA(&HSAI_RX, (uint8_t*)codec_rxbuf, codec_blocksize*AUDIO_CHANNELS*2);
#endif
  ASSERT(ret == HAL_OK, "Failed to start SAI DMA");
}

void Codec::pause(){
  HAL_SAI_DMAPause(&HSAI_RX);
  HAL_SAI_DMAPause(&HSAI_TX);
}

void Codec::resume(){
  HAL_SAI_DMAResume(&HSAI_RX);
  HAL_SAI_DMAResume(&HSAI_TX);
}

extern "C" {

// void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai){
// }

}

#endif /* USE_PCM3168A */

