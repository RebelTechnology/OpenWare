#include "Owl.h"
#include "OpenWareMidiControl.h"
#include "device.h"
#include "errorhandlers.h"

#include "HAL_MAX11300.h"

#ifdef USE_SCREEN
#include "Graphics.h"
Graphics graphics;

template<uint8_t SIZE>
void ParameterController<SIZE>::updateValue(uint8_t pid, int16_t value){
  // todo
}
#endif /* USE_SCREEN */

extern TIM_HandleTypeDef ENCODER_TIM1;

static bool updateMAX11300 = false;
static uint8_t portMode[20];
void setPortMode(uint8_t index, uint8_t mode){
  // todo: select range automatically based on output value
  if(index < 20){
    if(portMode[index] != mode){
      portMode[index] = mode;
      updateMAX11300 = true;
      // MAX11300_setDACValue(index+1, 0);
    }
  }
}
uint8_t getPortMode(uint8_t index){
  if(index < 20)
    return portMode[index];
  return PORT_UNI_INPUT;
}

void setup(){
    // Pixi
    extern SPI_HandleTypeDef MAX11300_SPI;
    MAX11300_init(&MAX11300_SPI);
    MAX11300_setDeviceControl(DCR_RESET);

      __HAL_TIM_SET_COUNTER(&ENCODER_TIM1, INT16_MAX/2);
  HAL_TIM_Encoder_Start_IT(&ENCODER_TIM1, TIM_CHANNEL_ALL);

  owl.setup();
}


// int16_t* Encoders_get(){
void updateEncoders(){
  static int16_t encoder_values[1] = {INT16_MAX/2};
  int16_t value = __HAL_TIM_GET_COUNTER(&ENCODER_TIM1);
  int16_t delta = value - encoder_values[0];
#ifdef USE_SCREEN
  if(delta)
    graphics.params.encoderChanged(0, delta);
#endif
  encoder_values[0] = value;
}


void loop(void){
  if(updateMAX11300){
    MAX11300_setDeviceControl(DCR_DACCTL_ImmUpdate|DCR_DACREF_Int|DCR_ADCCTL_ContSweep /* |DCR_ADCCONV_200ksps|DCR_BRST_Contextual*/);
    for(int i=0; i<20; ++i){
      uint16_t mode;
      switch(portMode[i]){
      case PORT_UNI_OUTPUT:
	mode = PCR_Range_DAC_0_P10|PCR_Mode_DAC;
	break;
      case PORT_UNI_INPUT:
      default:
	mode = PCR_Range_ADC_0_P10|PCR_Mode_ADC_SgEn_PosIn|PCR_ADCSamples_16|PCR_ADCref_INT;
	break;
      }
      MAX11300_setPortMode(i+1, mode);
    }
    updateMAX11300 = false;
  }
  
  MAX11300_bulkreadADC();
#ifdef USE_SCREEN
  for(int i=0; i<16; ++i){
    if(getPortMode(i) == PORT_UNI_INPUT){
      graphics.params.updateValue(i, MAX11300_getADCValue(i+1));
    }else{
      // DACs
    // TODO: store values set from patch somewhere and multiply with user[] value for outputs
    // graphics.params.updateOutput(i, getOutputValue(i));
      // MAX11300_setDACValue(i+1, graphics.params.parameters[i]);
      graphics.params.updateValue(i, 0);
      MAX11300_setDAC(i+1, graphics.params.parameters[i]);
    }
  }
  for(int i=16; i<20; ++i){
    if(getPortMode(i) == PORT_UNI_INPUT){
      graphics.params.updateValue(i, MAX11300_getADCValue(i+1));
    }else{
      graphics.params.updateValue(i, 0);
      MAX11300_setDAC(i+1, graphics.params.parameters[i]);
    }
  }
  updateEncoders();

  graphics.draw();
  graphics.display();
#endif /* USE_SCREEN */

  owl.loop();
}

// extern "C"{
// void usbd_audio_mute_callback(int16_t gain){
// }

// void usbd_audio_gain_callback(int16_t gain){
// }
// }