#include <stdint.h>
#include "device.h"
#include "Codec.h"
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
static const size_t rxhalf = CODEC_BUFFER_SIZE/2;
static const size_t rxfull = CODEC_BUFFER_SIZE;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
  extern TIM_HandleTypeDef htim8;
  if (htim == &htim8){
    // sample all channels
    iis3dwb_read();
#if defined USE_USBD_AUDIO && defined AUDIO_BYPASS
    // write directly to usb buffer
    audio_t* dst = audio_tx_buffer.getWriteHead(); // assume there's enough contiguous space for one full frame
#else
    audio_t* dst = codec_rxbuf + rxindex;
#endif
    *dst++ = dev_data_raw_acceleration.i16bit[0];
    *dst++ = dev_data_raw_acceleration.i16bit[1];
    *dst++ = dev_data_raw_acceleration.i16bit[2];
    // memcpy(dst, ads_samples, AUDIO_CHANNELS*sizeof(audio_t));
    // dst += ADS_ACTIVE_CHANNELS;
#if defined USE_USBD_AUDIO && defined AUDIO_BYPASS
    audio_tx_buffer.incrementWriteHead(AUDIO_CHANNELS);
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
