#include "cs4272.h"
#include "gpio.h"
#include "device.h"

static void NopDelay(uint32_t nops){
  while (nops--)	
    __asm("NOP");
}
#define delay(x) NopDelay(x*1000)

#define CS_TIMEOUT       1000

SPI_HandleTypeDef* hspi;

#define setCS()    setPin(CS_CS_GPIO_Port, CS_CS_Pin)
#define clearCS()  clearPin(CS_CS_GPIO_Port, CS_CS_Pin)

void codec_write(uint8_t reg, uint8_t data)
{
  /* In SPI mode, CS is the CS4271 chip select signal, CCLK is the control port bit clock, CDIN is the input data line from the microcontroller and the chip address is 0010000. All control signals are inputs and data is clocked in on the rising edge of CCLK. */

/* To write to a register, bring CS low. The first 7 bits on CDIN form the chip address, and must be 0010000. The eighth bit is a read/write indicator (R/W), which must be low to write. The next 8 bits form the Memory Address Pointer (MAP), which is set to the address of the register that is to be updated. The next 8 bits are the data which will be placed into the register designated by the MAP. */

  clearCS();	
  uint8_t buf[3];
  buf[0] = 0b00100000; // chip address, bit 0 is low to write
  buf[1] = reg & 0x0f;
  buf[2] = data;
  /* i2c_write(CODEC_ADDR,2,buf); */
  HAL_SPI_Transmit(hspi, buf, 3, CS_TIMEOUT);
  setCS();

/* The CS4271 has MAP auto increment capability, enabled by the INCR bit in the MAP. If INCR is 0, then the MAP will stay constant for successive writes. If INCR is set, then MAP will auto increment after each byte is written, allowing block writes to successive registers. */
}

void codec_init(SPI_HandleTypeDef* spi){

  hspi = spi;

  /* 1) When using the CS4271 with internally generated MCLK, hold RST low until
   * the power supply is stable. In this state, the Control Port is reset to its 
   * default settings. */
  clearPin(CS_RST_GPIO_Port, CS_RST_Pin);
  setPin(CS_CS_GPIO_Port, CS_CS_Pin);
  delay(10); // settle

  /* 2) Bring RST high. The device will remain in a low power state and the 
   * control port will be accessible. If internally generated MCLK is being 
   * used, it will appear on the MCLK pin prior to 1 ms from the release of RST. 
   */
  setPin(CS_RST_GPIO_Port, CS_RST_Pin);
  delay(1);
  /* If the CS4271 ever detects a high to low transition on AD0/CS after 
   * power-up, SPI mode will be selected. The Control Port registers are 
   * write-only in SPI mode. */
  clearPin(CS_CS_GPIO_Port, CS_CS_Pin);
  delay(1);

  /* 3) Write 03h to register 07h within 10 ms following the release of RST. 
   * This sets the Control Port Enable (CPEN) and Power Down (PDN) bits, 
   * activating the Control Port and placing the part in power-down. When using 
   * the CS4271 with internally generated MCLK, it is necessary to wait 1 ms 
   * following the release of RST before initiating this Control Port write. */
  codec_write(CODEC_MODE_CTRL2_REG, CODEC_MODE_CTRL2_POWER_DOWN
	      | CODEC_MODE_CTRL2_CTRL_PORT_EN);
	
  // Further setup
  delay(1);

  /* The desired register settings can be loaded while keeping the PDN bit set. */

  // functional mode 0b00: single speed mode
  // ratio 0b10: MCLK/LRCK = 512
  // crystal = MCLK = 24.576MHz, FS = LRCK = 48kHz (24.576MHz/512)
  codec_write(CODEC_MODE_CTRL1_REG, 
	      CODEC_MC_FUNC_MODE(0) |
	      CODEC_MC_RATIO_SEL(2) |
	      CODEC_MC_MASTER_SLAVE);


  /* codec_write(CODEC_DAC_VOL_REG, */
  /* 	      CODEC_DAC_VOL_CHB_CHA | */
  /* 	      CODEC_DAC_VOL_SOFT_RAMP(0) | */
  /* 	      CODEC_DAC_ZERO_CROSS | */
  /* 	      CODEC_DAC_VOL_ATAPI_DEFAULT); */

  // Release power down bit to start up codec
  codec_write(CODEC_MODE_CTRL2_REG, CODEC_MODE_CTRL2_CTRL_PORT_EN);
}

void codec_bypass(int bypass){
  uint8_t value = CODEC_MODE_CTRL2_CTRL_PORT_EN;
  if(bypass)
    value |= CODEC_MODE_CTRL2_LOOP;
  codec_write(CODEC_MODE_CTRL2_REG, value);
}

/* // set volume (negative for mute) */
/* void codec_set_volume(int8_t level){ */
/*   uint8_t value = codec_read(CODEC_DAC_CHA_VOL_REG); */
/*   value &= ~(CODEC_DAC_CHA_VOL_MUTE); */
/*   value |= CODEC_DAC_CHA_VOL_VOLUME(level); */
/*   codec_write(CODEC_DAC_CHA_VOL, value); */

/*   value = codec_read(CODEC_DAC_CHB_VOL_REG); */
/*   value &= ~(CODEC_DAC_CHB_VOL_MUTE); */
/*   value |= CODEC_DAC_CHB_VOL_VOLUME(level); */
/*   codec_write(CODEC_DAC_CHB_VOL, value); */
/* } */

