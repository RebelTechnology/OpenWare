#include "cs4272.h"
#include "gpio.h"
#include "device.h"

static void NopDelay(uint32_t nops){
  while (nops--)
    __asm("NOP");
}
#define delay(x) NopDelay(x*1000)
/* #define delay(x) HAL_Delay(x) */

#define CS_TIMEOUT       1000

static uint8_t volume;

#define setCS()    setPin(CS_CS_GPIO_Port, CS_CS_Pin)
#define clearCS()  clearPin(CS_CS_GPIO_Port, CS_CS_Pin)

void codec_write(uint8_t reg, uint8_t data){
  /* In SPI mode, CS is the CS4271 chip select signal, CCLK is the control port bit clock, CDIN is the input data line from the microcontroller and the chip address is 0010000. All control signals are inputs and data is clocked in on the rising edge of CCLK. */

/* To write to a register, bring CS low. The first 7 bits on CDIN form the chip address, and must be 0010000. The eighth bit is a read/write indicator (R/W), which must be low to write. The next 8 bits form the Memory Address Pointer (MAP), which is set to the address of the register that is to be updated. The next 8 bits are the data which will be placed into the register designated by the MAP. */

  clearCS();	
  uint8_t buf[3];
  buf[0] = 0b00100000; // chip address, bit 0 is low to write
  buf[1] = reg & 0x0f;
  buf[2] = data;
  /* i2c_write(CODEC_ADDR,2,buf); */
  extern SPI_HandleTypeDef CODEC_SPI;
  HAL_SPI_Transmit(&CODEC_SPI, buf, 3, CS_TIMEOUT);
  setCS();

/* The CS4271 has MAP auto increment capability, enabled by the INCR bit in the MAP. If INCR is 0, then the MAP will stay constant for successive writes. If INCR is set, then MAP will auto increment after each byte is written, allowing block writes to successive registers. */
}

void codec_reset(){
#ifdef CODEC_HP_FILTER
  codec_write(CODEC_ADC_CTRL, 0x10); // hp filters enabled, i2s data
#else
  codec_write(CODEC_ADC_CTRL, 0x10 | 0x03 ); // hp filters disabled
#endif
#ifdef CODEC_DAC_INVERT
  codec_write(CODEC_DAC_CTRL, CODEC_DAC_CTRL_INV_POL_AB); // invert DAC signal
#endif
}

void codec_init(){
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

  codec_write(CODEC_MODE_CTRL1_REG, (1<<3) | (1<<5) | 1); // i2s mode for DAC and ADC

  codec_write(CODEC_DAC_VOL_REG,
  	      CODEC_DAC_VOL_CHB_CHA |
  	      CODEC_DAC_SOFT_RAMP |
  	      /* CODEC_DAC_ZERO_CROSS | */
#ifdef CODEC_LR_SWAP
  	      CODEC_DAC_VOL_ATAPI_SWAP);
#else
              CODEC_DAC_VOL_ATAPI_DEFAULT);
#endif

  codec_reset();

  // Release power down bit to start up codec
  codec_write(CODEC_MODE_CTRL2_REG, CODEC_MODE_CTRL2_CTRL_PORT_EN);
}

void codec_bypass(int bypass){
  uint8_t value = CODEC_MODE_CTRL2_CTRL_PORT_EN;
  if(bypass)
    value |= CODEC_MODE_CTRL2_LOOP;
  codec_write(CODEC_MODE_CTRL2_REG, value);
}

void codec_mute(bool mute){  
  if(mute){
    codec_write(CODEC_DAC_CHA_VOL_REG, 0x80);
    codec_write(CODEC_DAC_CHB_VOL_REG, 0x80);
  }else{
    codec_write(CODEC_DAC_CHA_VOL_REG, volume);
    codec_write(CODEC_DAC_CHB_VOL_REG, volume);
  }
}

void codec_set_gain_in(int8_t level){
}

/* Set output gain between 0 (mute) and 127 (max) */
void codec_set_gain_out(int8_t level){
  /* The digital volume control allows the user to attenuate the signal in 1 dB increments from 0 to -127 dB.  */
  volume = 127-level;
  codec_write(CODEC_DAC_CHA_VOL_REG, volume);
  codec_write(CODEC_DAC_CHB_VOL_REG, volume);
}

