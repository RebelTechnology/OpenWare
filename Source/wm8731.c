#include "device.h"
#include "wm8731.h"
#include "errorhandlers.h"

void codec_write(uint8_t reg, uint16_t value){
/* void codec_write(uint8_t reg, uint8_t data){ */

  /* uint8_t Byte1 = ((RegisterAddr<<1)&0xFE) | ((RegisterValue>>8)&0x01); */
  /* uint8_t Byte2 = RegisterValue&0xFF; */

  /* Assemble 2-byte data in WM8731 format */
  uint8_t data[2] = {
    ((reg<<1)&0xFE) | ((value>>8)&0x01), value&0xFF   
  };

  extern I2C_HandleTypeDef hi2c2;
  HAL_StatusTypeDef ret = HAL_I2C_Master_Transmit(&hi2c2, CODEC_ADDRESS, data, 2, 10000);
  if(ret != HAL_OK)
    error(CONFIG_ERROR, "I2C transmit failed");
  /* HAL_I2C_Master_Transmit(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout) */
}

void codec_reset(){
  codec_write(RESET_CONTROL_REGISTER, 0); // reset
}

void codec_init(){
  /* Load default values */
  for(int i=0;i<WM8731_NUM_REGS-1;i++)
    codec_write(i, wm8731_init_data[i]);

  // set WM8731_MS master mode
  codec_write(DIGITAL_AUDIO_INTERFACE_FORMAT_REGISTER, WM8731_MS|WM8731_FORMAT_I2S|WM8731_IWL_16BIT);

  // clear OSCPD and OUTPD and CLKOUTPD
  codec_write(POWER_DOWN_CONTROL_REGISTER, WM8731_MICPD);

  // set active control
  codec_write(ACTIVE_CONTROL_REGISTER, WM8731_ACTIVE);
}

void codec_bypass(int bypass){
  uint16_t value = WM8731_MUTEMIC;
  if(bypass)
    value |= WM8731_BYPASS;
  else
    value |= WM8731_DACSEL;
  codec_write(ANALOGUE_AUDIO_PATH_CONTROL_REGISTER, value);
}

void codec_set_gain_out(int8_t level){
  uint16_t gain = (wm8731_init_data[LEFT_HEADPHONE_OUT_REGISTER] & 0x80) | (level & 0x7f);
  codec_write(LEFT_HEADPHONE_OUT_REGISTER, gain);
  gain = (wm8731_init_data[RIGHT_HEADPHONE_OUT_REGISTER] & 0x80) | (level & 0x7f);
  codec_write(RIGHT_HEADPHONE_OUT_REGISTER, gain);
}

/* Set input gain between 0 (mute) and 127 (max) */
void codec_set_gain_in(int8_t level){
  uint16_t gain = (wm8731_init_data[LEFT_LINE_IN_REGISTER] & 0xe0) | (level & 0x1f);
  codec_write(LEFT_LINE_IN_REGISTER, gain);
  gain = (wm8731_init_data[RIGHT_LINE_IN_REGISTER] & 0xe0) | (level & 0x1f);
  codec_write(RIGHT_LINE_IN_REGISTER, gain);
}

#if 0
/* void CodecController::clear(){ */
/*   memset(tx_buffer, 0, AUDIO_BUFFER_SIZE*sizeof(int16_t)); */
/*   memset(rx_buffer, 0, AUDIO_BUFFER_SIZE*sizeof(int16_t)); */
/* } */

void CodecController::init(ApplicationSettings& settings){
  // setActive(false);
  clear();

  /* configure codec */
  setSamplingRate(settings.audio_samplingrate);
  setCodecMaster(settings.audio_codec_master);
  setCodecProtocol((I2SProtocol)settings.audio_codec_protocol);
  setBitDepth(settings.audio_bitdepth);

  /* Configure the I2S peripheral */
  if(Codec_AudioInterface_Init(settings.audio_samplingrate, settings.audio_codec_master, 
			       settings.audio_codec_protocol, settings.audio_dataformat) != 0)
    assert_param(false);

  setInputGainLeft(settings.inputGainLeft);
  setInputGainRight(settings.inputGainRight);
  setOutputGainLeft(settings.outputGainLeft);
  setOutputGainRight(settings.outputGainRight);

  setBypass(settings.audio_codec_bypass);
  setSwapLeftRight(settings.audio_codec_swaplr);
  setHalfSpeed(settings.audio_codec_halfspeed);

  I2S_Block_Init(tx_buffer, rx_buffer, settings.audio_blocksize);
  // setActive(true);
}


uint32_t CodecController::getSamplingRate(){
  return settings.audio_samplingrate;
}

I2SProtocol CodecController::getProtocol(){
  return (I2SProtocol)settings.audio_codec_protocol;
}

bool CodecController::isMaster(){
  return settings.audio_codec_master;
}

void CodecController::setSamplingRate(uint32_t rate){
  switch(rate){
  case 8000:
    writeRegister(SAMPLING_CONTROL_REGISTER, WM8731_MODE_NORMAL|WM8731_SR_08_08);
    break;
  case 32000:
    writeRegister(SAMPLING_CONTROL_REGISTER, WM8731_MODE_NORMAL|WM8731_SR_32_32);
    break;
  case 48000:
    writeRegister(SAMPLING_CONTROL_REGISTER, WM8731_MODE_NORMAL|WM8731_SR_48_48);
    break;
  case 96000:
    writeRegister(SAMPLING_CONTROL_REGISTER, WM8731_MODE_NORMAL|WM8731_SR_96_96);
    // digital filter type 2, BOSR 0 128fs
    break;
  }
}

void CodecController::setActive(bool active){
  /* It is recommended that between changing any content of Digital Audio Interface or Sampling Control Register that the active bit is reset then set. */
  if(active){
    /* OUTPD should be set AFTER active control, but that seems to hang the codec */
    clearRegister(POWER_DOWN_CONTROL_REGISTER, WM8731_OUTPD);
    setRegister(ACTIVE_CONTROL_REGISTER, WM8731_ACTIVE);
  }else{
    setRegister(POWER_DOWN_CONTROL_REGISTER, WM8731_OUTPD);
    clearRegister(ACTIVE_CONTROL_REGISTER, WM8731_ACTIVE);
  }
}

void CodecController::setCodecMaster(bool master){
  if(master){
    clearRegister(POWER_DOWN_CONTROL_REGISTER, WM8731_OSCPD);
    setRegister(DIGITAL_AUDIO_INTERFACE_FORMAT_REGISTER, WM8731_MS);
  }else{
    setRegister(POWER_DOWN_CONTROL_REGISTER, WM8731_OSCPD);
    clearRegister(DIGITAL_AUDIO_INTERFACE_FORMAT_REGISTER, WM8731_MS);
  }
}

void CodecController::setCodecProtocol(I2SProtocol protocol){
  if(protocol == I2S_PROTOCOL_PHILIPS){
    writeRegister(DIGITAL_AUDIO_INTERFACE_FORMAT_REGISTER, 
		  (wm8731_registers[DIGITAL_AUDIO_INTERFACE_FORMAT_REGISTER] & 0x1fc)
		  | WM8731_FORMAT_I2S);
  }else{
    writeRegister(DIGITAL_AUDIO_INTERFACE_FORMAT_REGISTER, 
		  (wm8731_registers[DIGITAL_AUDIO_INTERFACE_FORMAT_REGISTER] & 0x1fc)
		  | WM8731_FORMAT_MSB_LJ);
  }
}

void CodecController::setBitDepth(uint8_t bits){
  switch(bits){
  case 16:
    writeRegister(DIGITAL_AUDIO_INTERFACE_FORMAT_REGISTER, 
		  (wm8731_registers[DIGITAL_AUDIO_INTERFACE_FORMAT_REGISTER] & ~(3<<2))
		  | WM8731_IWL_16BIT);
    break;
  case 24:
    writeRegister(DIGITAL_AUDIO_INTERFACE_FORMAT_REGISTER, 
		  (wm8731_registers[DIGITAL_AUDIO_INTERFACE_FORMAT_REGISTER] & ~(3<<2))
		  | WM8731_IWL_24BIT);
    break;
  case 32:
    writeRegister(DIGITAL_AUDIO_INTERFACE_FORMAT_REGISTER, 
		  (wm8731_registers[DIGITAL_AUDIO_INTERFACE_FORMAT_REGISTER] & ~(3<<2))
		  | WM8731_IWL_32BIT);
    break;
  }
}

void CodecController::writeRegister(uint8_t addr, uint16_t value){
  if(addr < WM8731_NUM_REGS)
    wm8731_registers[addr] = value;
  Codec_WriteRegister(addr, value);
}

uint16_t CodecController::readRegister(uint8_t addr){
  return wm8731_registers[addr];
}

void CodecController::setRegister(uint8_t addr, uint16_t value){
  writeRegister(addr, wm8731_registers[addr] | value);
}

void CodecController::clearRegister(uint8_t addr, uint16_t value){
  writeRegister(addr, wm8731_registers[addr] & ~value);
}

void CodecController::start(){
//   setPin(GPIOA, GPIO_Pin_7); // PA7 DEBUG
  setActive(true);
  if(isMaster()){
    /* See STM32F405 Errata, I2S device limitations */
    /* The I2S peripheral must be enabled when the external master sets the WS line at: */
    if(getProtocol() == I2S_PROTOCOL_PHILIPS){
      while(getPin(CODEC_I2S_GPIO, CODEC_I2S_WS_PIN)); // wait for low
      /* High level when the I2S protocol is selected. */
      while(!getPin(CODEC_I2S_GPIO, CODEC_I2S_WS_PIN)); // wait for high
    }else{
      while(!getPin(CODEC_I2S_GPIO, CODEC_I2S_WS_PIN)); // wait for high
      /* Low level when the LSB or MSB-justified mode is selected. */
      while(getPin(CODEC_I2S_GPIO, CODEC_I2S_WS_PIN)); // wait for low
    }
  }
  I2S_Enable();
  I2S_Run();
}

void CodecController::stop(){
  I2S_Disable();
//   clearPin(GPIOA, GPIO_Pin_7); // PA7 DEBUG
  setActive(false);
}

void CodecController::pause(){
  I2S_Pause();
}

void CodecController::resume(){
  I2S_Resume();
}

void CodecController::softMute(bool mute){
  if(mute)
    setRegister(DIGITAL_AUDIO_PATH_CONTROL_REGISTER, WM8731_DACMU);
  else
    clearRegister(DIGITAL_AUDIO_PATH_CONTROL_REGISTER, WM8731_DACMU);
}

void CodecController::setBypass(bool bypass){
  if(bypass){
    softMute(true);
    writeRegister(ANALOGUE_AUDIO_PATH_CONTROL_REGISTER, 
		  (wm8731_registers[ANALOGUE_AUDIO_PATH_CONTROL_REGISTER] & ~WM8731_DACSEL) | WM8731_BYPASS);
  }else{
    writeRegister(ANALOGUE_AUDIO_PATH_CONTROL_REGISTER, 
		  (wm8731_registers[ANALOGUE_AUDIO_PATH_CONTROL_REGISTER] & ~WM8731_BYPASS) | WM8731_DACSEL);
    softMute(false);
  }
}

bool CodecController::getBypass(){
  return wm8731_registers[ANALOGUE_AUDIO_PATH_CONTROL_REGISTER] & WM8731_BYPASS;
}

void CodecController::setInputGainLeft(uint8_t gain){
  gain = (wm8731_registers[LEFT_LINE_IN_REGISTER] & 0xe0) | (gain & 0x1f);
  writeRegister(LEFT_LINE_IN_REGISTER, gain);
}

void CodecController::setInputGainRight(uint8_t gain){
  gain = (wm8731_registers[RIGHT_LINE_IN_REGISTER] & 0xe0) | (gain & 0x1f);
  writeRegister(RIGHT_LINE_IN_REGISTER, gain);
}

void CodecController::setOutputGainLeft(uint8_t gain){
  gain = (wm8731_registers[LEFT_HEADPHONE_OUT_REGISTER] & 0x80) | (gain & 0x7f);
  writeRegister(LEFT_HEADPHONE_OUT_REGISTER, gain);
}

void CodecController::setOutputGainRight(uint8_t gain){
  gain = (wm8731_registers[RIGHT_HEADPHONE_OUT_REGISTER] & 0x80) | (gain & 0x7f);
  writeRegister(RIGHT_HEADPHONE_OUT_REGISTER, gain);
}

uint8_t CodecController::getInputGainLeft(){
  return wm8731_registers[LEFT_LINE_IN_REGISTER] & 0x1f;
}

uint8_t CodecController::getInputGainRight(){
  return wm8731_registers[RIGHT_LINE_IN_REGISTER] & 0x1f;
}

uint8_t CodecController::getOutputGainLeft(){
  return wm8731_registers[LEFT_HEADPHONE_OUT_REGISTER] & 0x7f;
}

uint8_t CodecController::getOutputGainRight(){
  return wm8731_registers[RIGHT_HEADPHONE_OUT_REGISTER] & 0x7f;
}

void CodecController::setInputMuteLeft(bool mute){
  if(mute)
    setRegister(LEFT_LINE_IN_REGISTER, WM8731_INMUTE);
  else
    clearRegister(LEFT_LINE_IN_REGISTER, WM8731_INMUTE);
}

void CodecController::setInputMuteRight(bool mute){
  if(mute)
    setRegister(RIGHT_LINE_IN_REGISTER, WM8731_INMUTE);
  else
    clearRegister(RIGHT_LINE_IN_REGISTER, WM8731_INMUTE);
}

void CodecController::setOutputMuteLeft(bool mute){
  if(mute)
    setRegister(LEFT_HEADPHONE_OUT_REGISTER, WM8731_HPVOL_MUTE);
  else
    clearRegister(LEFT_HEADPHONE_OUT_REGISTER, WM8731_HPVOL_MUTE);
}

void CodecController::setOutputMuteRight(bool mute){
  if(mute)
    setRegister(RIGHT_HEADPHONE_OUT_REGISTER, WM8731_HPVOL_MUTE);
  else
    clearRegister(RIGHT_HEADPHONE_OUT_REGISTER, WM8731_HPVOL_MUTE);
}

bool CodecController::getInputMuteLeft(){
  return wm8731_registers[LEFT_LINE_IN_REGISTER] & WM8731_INMUTE;
}

bool CodecController::getInputMuteRight(){
  return wm8731_registers[RIGHT_LINE_IN_REGISTER] & WM8731_INMUTE;
}

bool CodecController::getOutputMuteLeft(){
  return wm8731_registers[LEFT_HEADPHONE_OUT_REGISTER] & WM8731_HPVOL_MUTE;
}

bool CodecController::getOutputMuteRight(){
  return wm8731_registers[RIGHT_HEADPHONE_OUT_REGISTER] & WM8731_HPVOL_MUTE;
}

void CodecController::setSwapLeftRight(bool swap){
  if(swap)
    setRegister(DIGITAL_AUDIO_INTERFACE_FORMAT_REGISTER, WM8731_LRSWAP);
  else
    clearRegister(DIGITAL_AUDIO_INTERFACE_FORMAT_REGISTER, WM8731_LRSWAP);
}

bool CodecController::getSwapLeftRight(){
  return wm8731_registers[DIGITAL_AUDIO_INTERFACE_FORMAT_REGISTER] & WM8731_LRSWAP;
}

void CodecController::setHalfSpeed(bool half){
  if(half)
    setRegister(SAMPLING_CONTROL_REGISTER, WM8731_CLKIDIV2);
  else
    clearRegister(SAMPLING_CONTROL_REGISTER, WM8731_CLKIDIV2);
}

bool CodecController::getHalfSpeed(){
  return wm8731_registers[SAMPLING_CONTROL_REGISTER] & WM8731_CLKIDIV2;
}

#endif // 0
