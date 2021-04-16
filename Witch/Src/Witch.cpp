#include <string.h>
#include <stdlib.h>
#include "device.h"
#include "Owl.h"
#include "errorhandlers.h"
#include "message.h"
#include "ProgramManager.h"
#include "PatchRegistry.h"
#include "OpenWareMidiControl.h"
#include "Codec.h"
#include "ApplicationSettings.h"
#include "qint.h"

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef abs
#define abs(x) ((x)>0?(x):-(x))
#endif

template<size_t SIZE, typename value_t>
class TakeoverControls {
private:
  value_t values[SIZE];
  bool takeover[SIZE];
public:
  TakeoverControls(){
    reset(true);
  }
  value_t get(uint8_t index){
    return values[index];
  }
  void set(uint8_t index, value_t value){
    values[index] = value;
  }
  void update(uint8_t index, value_t value, value_t threshold){
    if(takeover[index]){
      values[index] = value;
    }else if(abs(values[index] - value) < threshold){
      takeover[index] = true;
      values[index] = value;
    }
  }
  bool taken(uint8_t index){
    return takeover[index];
  }
  void reset(uint8_t index, bool state){
    takeover[index] = state;
  }
  void reset(bool state){
    for(size_t i=0; i<SIZE; ++i)
      takeover[i] = state;
  }
};

TakeoverControls<10, int16_t> takeover;
int16_t dac_values[2];

void onChangePin(uint16_t pin){
  if(owl.getOperationMode() == RUN_MODE){
    switch(pin){
    case SW2_Pin:
      {
	bool state = HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin) == GPIO_PIN_RESET;
	setButtonValue(BUTTON_B, state);
	break;
      }
    case SW3_Pin:
      {
	bool state = HAL_GPIO_ReadPin(SW3_GPIO_Port, SW3_Pin) == GPIO_PIN_RESET;
	setButtonValue(BUTTON_C, state);
	break;
      }
    case SW4_Pin:
      {
	bool state = HAL_GPIO_ReadPin(SW4_GPIO_Port, SW4_Pin) == GPIO_PIN_RESET;
	setButtonValue(BUTTON_D, state);
	break;
      }
    }
  }
}

void setAnalogValue(uint8_t ch, int16_t value){
  if(owl.getOperationMode() == RUN_MODE){
    extern DAC_HandleTypeDef hdac;
    value = __USAT(value, 12);
    switch(ch){
    case PARAMETER_F:
      HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, value);
      dac_values[0] = value;
      break;
    case PARAMETER_G:
      HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, __USAT(value, 12));
      dac_values[1] = value;
      break;
    }
  }
}

void setGateValue(uint8_t ch, int16_t value){
  if(owl.getOperationMode() == RUN_MODE){
    switch(ch){
      // todo: always fiddle pushbutton LEDs to allow toggle
      // case BUTTON_A:
      //   setLed(1, value);
      //   break;
      // case BUTTON_B:
      //   setLed(2, value);
      //   break;
      // case BUTTON_C:
      //   setLed(3, value);
      //   break;
      // case BUTTON_D:
      //   setLed(f, value);
      //   break;
    case PUSHBUTTON:
    case BUTTON_E:
      HAL_GPIO_WritePin(TR_OUT1_GPIO_Port, TR_OUT1_Pin, value ? GPIO_PIN_RESET :  GPIO_PIN_SET);
      break;
    case BUTTON_F:
      HAL_GPIO_WritePin(TR_OUT2_GPIO_Port, TR_OUT2_Pin, value ? GPIO_PIN_RESET :  GPIO_PIN_SET);
      break;
    }
  }
}

void initLed(){
  extern TIM_HandleTypeDef htim1;
  extern TIM_HandleTypeDef htim3;
  extern TIM_HandleTypeDef htim4;
  extern TIM_HandleTypeDef htim13;
  HAL_TIM_Base_Start(&htim1);
  HAL_TIM_Base_Start(&htim3);
  HAL_TIM_Base_Start(&htim4);
  HAL_TIM_Base_Start(&htim13);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim13, TIM_CHANNEL_1);
}

void setLed(uint8_t led, uint32_t rgb){
  uint32_t value = 1023 - (__USAT(rgb>>2, 10)); // expects 12-bit parameter value
  switch(led){
  case 0:
    break;
  case 1:
    TIM3->CCR1 = value;
    break;
  case 2:
    TIM3->CCR3 = value;
    break;
  case 3:
    TIM4->CCR3 = value;
    break;
  case 4:
    TIM13->CCR1 = value;
    break;
  case 5:
    TIM3->CCR4 = value;
    break;
  case 6:
    TIM1->CCR1 = value;
    break;
  }
}

bool isModeButtonPressed(){
  return HAL_GPIO_ReadPin(SW5_GPIO_Port, SW5_Pin) == GPIO_PIN_RESET;
}

int16_t getAttenuatedCV(uint8_t index, uint16_t* adc_values){
  return Q15_MUL_Q15(adc_values[index*2], takeover.get(index+5)<<1);
}

void updateParameters(int16_t* parameter_values, size_t parameter_len, uint16_t* adc_values, size_t adc_len){
  // cv inputs are ADC_A, C, E, G
  // knobs are ADC_B, D, F, H, I
  if(isModeButtonPressed()){
    for(size_t i=0; i<4; ++i){
      takeover.update(i+5, adc_values[i*2+1], 31);
      if(takeover.taken(i+5))
        setLed(i+1, 0);
      else
        setLed(i+1, 4095);
    }
    takeover.update(9, adc_values[ADC_I], 31);
    if(takeover.taken(9)){
      setLed(5, 0);
      setLed(6, 0);
    }else{
      setLed(5, 4095);
      setLed(6, 4095);
    }      
  }else{
    for(size_t i=0; i<4; ++i){
      takeover.update(i, adc_values[i*2+1], 31);
      if(takeover.taken(i))
        setLed(i+1, getAttenuatedCV(i, adc_values));
      else
        setLed(i+1, 4095);
    }
    takeover.update(4, adc_values[ADC_I], 31);
    if(!takeover.taken(4)){
      setLed(5, 4095);
      setLed(6, 4095);
    }else{
      setLed(5, dac_values[0]);
      setLed(6, dac_values[1]);
    }
    // IIR exponential filter with lambda 0.75
    for(size_t i=0; i<4; ++i){
      int16_t xn = parameter_values[i]*3;
      int16_t x = takeover.get(i);
      int16_t cv = getAttenuatedCV(i, adc_values);
      parameter_values[i] = __USAT((xn+x+cv)>>2, 12);
    }
    parameter_values[4] = __USAT((parameter_values[4]*3+takeover.get(4))>>2, 12);
  }
}


/**
  * @brief  Configure gpio mode for a dedicated pin on dedicated port.
  * @note   I/O mode can be Input mode, General purpose output, Alternate function mode or Analog.
  * @note   Warning: only one pin can be passed as parameter.
  * @rmtoll MODER        MODEy         LL_GPIO_SetPinMode
  * @param  GPIOx GPIO Port
  * @param  Pin This parameter can be one of the following values:
  *         @arg @ref LL_GPIO_PIN_0
  *         @arg @ref LL_GPIO_PIN_1
  *         @arg @ref LL_GPIO_PIN_2
  *         @arg @ref LL_GPIO_PIN_3
  *         @arg @ref LL_GPIO_PIN_4
  *         @arg @ref LL_GPIO_PIN_5
  *         @arg @ref LL_GPIO_PIN_6
  *         @arg @ref LL_GPIO_PIN_7
  *         @arg @ref LL_GPIO_PIN_8
  *         @arg @ref LL_GPIO_PIN_9
  *         @arg @ref LL_GPIO_PIN_10
  *         @arg @ref LL_GPIO_PIN_11
  *         @arg @ref LL_GPIO_PIN_12
  *         @arg @ref LL_GPIO_PIN_13
  *         @arg @ref LL_GPIO_PIN_14
  *         @arg @ref LL_GPIO_PIN_15
  * @param  Mode This parameter can be one of the following values:
  *         @arg @ref LL_GPIO_MODE_INPUT
  *         @arg @ref LL_GPIO_MODE_OUTPUT
  *         @arg @ref LL_GPIO_MODE_ALTERNATE
  *         @arg @ref LL_GPIO_MODE_ANALOG
  * @retval None
  */
#define LL_GPIO_PIN_0                      GPIO_BSRR_BS_0 /*!< Select pin 0 */
#define LL_GPIO_PIN_1                      GPIO_BSRR_BS_1 /*!< Select pin 1 */
#define LL_GPIO_PIN_2                      GPIO_BSRR_BS_2 /*!< Select pin 2 */
#define LL_GPIO_PIN_3                      GPIO_BSRR_BS_3 /*!< Select pin 3 */
#define LL_GPIO_PIN_4                      GPIO_BSRR_BS_4 /*!< Select pin 4 */
#define LL_GPIO_PIN_5                      GPIO_BSRR_BS_5 /*!< Select pin 5 */
#define LL_GPIO_PIN_6                      GPIO_BSRR_BS_6 /*!< Select pin 6 */
#define LL_GPIO_PIN_7                      GPIO_BSRR_BS_7 /*!< Select pin 7 */
#define LL_GPIO_PIN_8                      GPIO_BSRR_BS_8 /*!< Select pin 8 */
#define LL_GPIO_PIN_9                      GPIO_BSRR_BS_9 /*!< Select pin 9 */
#define LL_GPIO_PIN_10                     GPIO_BSRR_BS_10 /*!< Select pin 10 */
#define LL_GPIO_PIN_11                     GPIO_BSRR_BS_11 /*!< Select pin 11 */
#define LL_GPIO_PIN_12                     GPIO_BSRR_BS_12 /*!< Select pin 12 */
#define LL_GPIO_PIN_13                     GPIO_BSRR_BS_13 /*!< Select pin 13 */
#define LL_GPIO_PIN_14                     GPIO_BSRR_BS_14 /*!< Select pin 14 */
#define LL_GPIO_PIN_15                     GPIO_BSRR_BS_15 /*!< Select pin 15 */

#define LL_GPIO_MODE_INPUT                 (0x00000000U) /*!< Select input mode */
#define LL_GPIO_MODE_OUTPUT                GPIO_MODER_MODER0_0  /*!< Select output mode */
#define LL_GPIO_MODE_ALTERNATE             GPIO_MODER_MODER0_1  /*!< Select alternate function mode */
#define LL_GPIO_MODE_ANALOG                GPIO_MODER_MODER0    /*!< Select analog mode */

__STATIC_INLINE void LL_GPIO_SetPinMode(GPIO_TypeDef *GPIOx, uint32_t Pin, uint32_t Mode)
{
  MODIFY_REG(GPIOx->MODER, (GPIO_MODER_MODER0 << (POSITION_VAL(Pin) * 2U)), (Mode << (POSITION_VAL(Pin) * 2U)));
}
__STATIC_INLINE uint32_t LL_GPIO_IsInputPinSet(GPIO_TypeDef *GPIOx, uint32_t PinMask)
{
  return (READ_BIT(GPIOx->IDR, PinMask) == (PinMask));
}
__STATIC_INLINE uint32_t LL_GPIO_ReadInputPort(GPIO_TypeDef *GPIOx)
{
  return (uint32_t)(READ_REG(GPIOx->IDR));
}
__STATIC_INLINE uint32_t LL_GPIO_ReadOutputPort(GPIO_TypeDef *GPIOx)
{
  return (uint32_t)(READ_REG(GPIOx->ODR));
}
__STATIC_INLINE void LL_GPIO_WriteOutputPort(GPIO_TypeDef *GPIOx, uint32_t PortValue)
{
  WRITE_REG(GPIOx->ODR, PortValue);
}

#define SW1_LL_Pin LL_GPIO_PIN_2
#define SW2_LL_Pin LL_GPIO_PIN_1
#define SW3_LL_Pin LL_GPIO_PIN_2
#define SW4_LL_Pin LL_GPIO_PIN_9

bool fiddle(int i, bool selected){
  GPIO_TypeDef* port;
  uint16_t pin;
  uint32_t llpin;
  switch(i){
  case 1: // PB2
    port = SW1_GPIO_Port;
    pin = SW1_Pin;
    llpin = SW1_LL_Pin;
    break;
  case 2: // PB1
    port = SW2_GPIO_Port;
    pin = SW2_Pin;
    llpin = SW2_LL_Pin;
    break;
  case 3: // PD2
    port = SW3_GPIO_Port;
    pin = SW3_Pin;
    llpin = SW3_LL_Pin;
    break;
  case 4: // PB9
    port = SW4_GPIO_Port;
    pin = SW4_Pin;
    llpin = SW4_LL_Pin;
    break;
  }
  LL_GPIO_SetPinMode(port, llpin, LL_GPIO_MODE_INPUT);
  // bool value = LL_GPIO_IsInputPinSet(port, llpin);
  bool value = HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_RESET;
  // uint32_t portvalues = LL_GPIO_ReadOutputPort(port);
  if(selected){
    LL_GPIO_SetPinMode(port, llpin, LL_GPIO_MODE_OUTPUT);
    // LL_GPIO_WriteOutputPort(port, portvalues & ~llpin); // switch pin off
    // LL_GPIO_WriteOutputPort(port, port | llpin); // switch pin on
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
  }
  return value;

}

#define PATCH_RESET_COUNTER (600/MAIN_LOOP_SLEEP_MS)

static uint32_t counter = 0;
static void update_preset(){
  switch(owl.getOperationMode()){
  case STARTUP_MODE:
  case STREAM_MODE:
  case LOAD_MODE: {
    uint32_t value = counter & 0xfff;    
    setLed(1, counter > PATCH_RESET_COUNTER*0.1 ? value : 0);
    setLed(2, counter > PATCH_RESET_COUNTER*0.2 ? value : 0);
    setLed(5, counter > PATCH_RESET_COUNTER*0.3 ? value : 0);
    setLed(6, counter > PATCH_RESET_COUNTER*0.4 ? value : 0);
    setLed(3, counter > PATCH_RESET_COUNTER*0.5 ? value : 0);
    setLed(4, counter > PATCH_RESET_COUNTER*0.6 ? value : 0);
    if(getErrorStatus() != NO_ERROR || isModeButtonPressed())
      owl.setOperationMode(ERROR_MODE);
    break;
  }
  case RUN_MODE:
    if(isModeButtonPressed()){
      takeover.reset(false);
      owl.setOperationMode(CONFIGURE_MODE);
    }else if(getErrorStatus() != NO_ERROR){
      owl.setOperationMode(ERROR_MODE);
    }else{
      // setLed(1, Q15_MUL_Q15(getAnalogValue(ADC_A), takeover.get(5)));
      // setLed(2, Q15_MUL_Q15(getAnalogValue(ADC_C), takeover.get(6)));
      // setLed(3, Q15_MUL_Q15(getAnalogValue(ADC_E), takeover.get(7)));
      // setLed(4, Q15_MUL_Q15(getAnalogValue(ADC_G), takeover.get(8)));
    }
    break;
  case CONFIGURE_MODE:
    if(isModeButtonPressed()){
      uint8_t patchselect = program.getProgramIndex();
      for(int i=1; i<=4; ++i){
	if(fiddle(i, i==patchselect))
	  patchselect = i;
      }
      if(patchselect >= registry.getNumberOfPatches())
	patchselect = program.getProgramIndex();
      if(program.getProgramIndex() != patchselect){
	program.loadProgram(patchselect); // enters load mode
	program.resetProgram(false);
	owl.setOperationMode(CONFIGURE_MODE);
	// reset CV outputs to initial values
	setAnalogValue(PARAMETER_F, 0);
	setAnalogValue(PARAMETER_G, 0);
      }
      if(takeover.taken(9)){
	int16_t value = takeover.get(9)>>5;
	settings.audio_output_gain = value;
	codec.setOutputGain(value);
      }
    }else{
      for(int i=1; i<=4; ++i)
	fiddle(i, false); // set all to input mode
      takeover.reset(false);
      owl.setOperationMode(RUN_MODE);
      // reset CV outputs to previous values
      setAnalogValue(PARAMETER_F, dac_values[0]);
      setAnalogValue(PARAMETER_G, dac_values[1]);
    }
    break;
  case ERROR_MODE:
    setLed(1, counter > PATCH_RESET_COUNTER*0.5 ? 4095 : 0);
    setLed(2, counter > PATCH_RESET_COUNTER*0.5 ? 4095 : 0);
    setLed(5, counter < PATCH_RESET_COUNTER*0.5 ? 4095 : 0);
    setLed(6, counter < PATCH_RESET_COUNTER*0.5 ? 4095 : 0);
    setLed(3, counter > PATCH_RESET_COUNTER*0.5 ? 4095 : 0);
    setLed(4, counter > PATCH_RESET_COUNTER*0.5 ? 4095 : 0);
    if(isModeButtonPressed()){
      setErrorStatus(NO_ERROR);
      owl.setOperationMode(RUN_MODE); // allows new patch selection if patch doesn't load
      program.resetProgram(false);
    }
    break;
  }
  if(++counter >= PATCH_RESET_COUNTER)
    counter = 0;
}

void onChangeMode(OperationMode new_mode, OperationMode old_mode){
  for(int i=1; i<=6; ++i)
    setLed(i, 0);
  setGateValue(BUTTON_E, 0);
  setGateValue(BUTTON_F, 0);
  counter = 0;
}

void setup(){
  initLed();
  HAL_GPIO_WritePin(LEDPWM_GPIO_Port, LEDPWM_Pin, GPIO_PIN_SET);
  owl.setup();
  for(size_t i=5; i<9; ++i)
    takeover.set(i, 2048); // set CV attenuation to 1
  takeover.set(9, settings.audio_output_gain<<5);
}

void loop(void){
  MX_USB_HOST_Process(); // todo: enable PWR management
  if(owl.getOperationMode() == RUN_MODE){
    bool state = HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin) == GPIO_PIN_RESET;
    if(state != getButtonValue(BUTTON_A)){
      setButtonValue(PUSHBUTTON, state);
      setButtonValue(BUTTON_A, state);
    }
  }
  update_preset();
  owl.loop();
}
