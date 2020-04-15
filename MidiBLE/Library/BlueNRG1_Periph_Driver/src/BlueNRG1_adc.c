/**
******************************************************************************
* @file    BlueNRG1_adc.c
* @author  VMA Application Team
* @version V2.1.0
* @date    21-March-2016
* @brief   This file provides all the ADC firmware functions.
******************************************************************************
* @attention
*
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
* TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
* FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
* <h2><center>&copy; COPYRIGHT 2016 STMicroelectronics</center></h2>
******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "BlueNRG1_adc.h"


/** @addtogroup BLUENRG1_Peripheral_Driver BLUENRG1 Peripheral Driver
  * @{
  */

/** @addtogroup ADC_Peripheral  ADC Peripheral
  * @{
  */


/** @defgroup ADC_Private_Types Private Types
* @{
*/

/**
* @}
*/


/** @defgroup ADC_Private_Constants Private Constants
* @{
*/
#define FS_SIGMA_DELTA          ((float)(2.4))


/**
* @}
*/


/** @defgroup ADC_Private_Macros Private Macros
* @{
*/

/**
* @}
*/

/** @defgroup ADC_Private_Variables Private Variables
* @{
*/

/**
* @}
*/

/** @defgroup ADC_Private_Function Private Functions
* @{
*/


/**
* @brief  Convert raw ADC value in converted differential voltage value.
* @param  raw_value: ADC raw value
* @retval float: converted differential voltage value
*/
float ADC_ConvertDifferentialVoltage(int32_t raw_value, uint8_t attenuation)
{
  float pga = 0;
  
  switch(attenuation) {
  case ADC_Attenuation_0dB:
    pga = 0;
    break;
  case ADC_Attenuation_6dB02:
    pga = 1;
    break;
  case ADC_Attenuation_9dB54:
    pga = 2;
    break;
  }
  
  float divider, kadc;
  if(ADC->CONF_b.ROUND16 == SET) {
    divider = 65536.0;
    kadc = 1.12;
  }
  else {
    divider = 2147483648.0;
    kadc = 1.68;
  }

  return (kadc * ( (pga + 1) * FS_SIGMA_DELTA )*(raw_value / divider));
}

/**
* @brief  Convert raw ADC value in converted single ended voltage value.
* @param  raw_value ADC raw value
* @param  channel: the selected single channel type
* @param  vRef: the configured reference voltage
* @retval float: converted single ended voltage value
*/
float ADC_ConvertSingleEndedVoltage(int32_t raw_value, uint8_t channel, uint8_t vRef, uint8_t attenuation)
{
  float pga = 0, vref = 0.0;
  float divider, kadc;
  
  switch(vRef) {
  case ADC_ReferenceVoltage_0V:
    vref = 0.0;
    break;
  case ADC_ReferenceVoltage_0V4:
    vref = 0.4;
    break;
  case ADC_ReferenceVoltage_0V6:
    vref = 0.6;
    break;
  case ADC_ReferenceVoltage_1V2:
    vref = 1.2;
    break;
  }
  
  switch(attenuation) {
  case ADC_Attenuation_0dB:
    pga = 0;
    break;
  case ADC_Attenuation_6dB02:
    pga = 1;
    break;
  case ADC_Attenuation_9dB54:
    pga = 2;
    break;
  }
  
  if(ADC->CONF_b.ROUND16 == SET) {
    divider = 16384.0;
    kadc = 1.12;
  }
  else {
    divider = 536870912.0;
    kadc = 1.68;
  }
  
  if(channel == ADC_Input_AdcPin1) 
    return  ((((pga + 1) * FS_SIGMA_DELTA) / 4.0) * (1 + kadc * (raw_value/divider)) + (vref - 0.6) * (pga + 1));
  else
    return  ((((pga + 1) * FS_SIGMA_DELTA) / 4.0) * (1 - kadc * (raw_value/divider)) + (vref - 0.6) * (pga + 1));
}


/**
* @brief  Convert raw ADC value in battery level.
* @param  raw_value ADC raw value
* @param  vRef: the configured reference voltage
* @retval float: converted battery level
*/
float ADC_ConvertBatterySensor(int32_t raw_value, uint8_t vRef)
{
  return ((4.36 * (ADC_ConvertSingleEndedVoltage(raw_value, ADC_Input_AdcPin2, vRef, ADC_Attenuation_0dB) - 0.622)) + 2.7);
}

/**
* @brief  Convert raw ADC value in temperature level.
* @param  raw_value ADC raw value
* @param  vRef: the configured reference voltage
* @retval float: converted temperature level
*/
float ADC_ConvertTemperatureSensor(int32_t raw_value, uint8_t vRef, uint8_t attenuation)
{
  return  ((410 * (ADC_ConvertSingleEndedVoltage(raw_value, ADC_Input_AdcPin2, vRef, ADC_Attenuation_0dB) - 0.735)) + 27.5);
}


/**
* @}
*/


/** @defgroup ADC_Public_Functions Public Functions
* @{
*/

/**
* @brief  Deinitializes ADC peripheral registers to their default reset values.
* @param  None
* @retval None
*/
void ADC_DeInit(void)
{
  /* Enable ADC reset state */
  ADC->CTRL_b.RESET = SET;
}


/**
  * @brief  Fills the ADC_InitStruct with default values.
  * @param  ADC_InitStruct: pointer to an @ref ADC_InitType structure which will be initialized.
  * @retval None
  */
void ADC_StructInit(ADC_InitType* ADC_InitStruct)
{
  /* Set the decimation rate */
  ADC_InitStruct->ADC_DecimationRate = ADC_DecimationRate_200;

  /* Select the input source */
  ADC_InitStruct->ADC_Input = ADC_Input_None;

  /* Set the reference voltage */
  ADC_InitStruct->ADC_ReferenceVoltage = ADC_ReferenceVoltage_1V2;

  /* Set the conversion mode */
  ADC_InitStruct->ADC_ConversionMode = ADC_ConversionMode_Single;

  /* Set the attenuation */
  ADC_InitStruct->ADC_Attenuation = ADC_Attenuation_0dB;
}


/**
* @brief  Initializes the ADC peripheral according to the specified parameters
*         in the ADC_InitStruct.
* @param  ADC_InitStruct: pointer to an @ref ADC_InitType structure that contains 
*         the configuration information for the ADC peripheral.
* @retval None
*/
void ADC_Init(ADC_InitType* ADC_InitStruct)               
{
  /* Check the parameters */
  assert_param(IS_ADC_DECIMATIONRATE(ADC_InitStruct->ADC_DecimationRate)); 
  assert_param(IS_ADC_INPUT(ADC_InitStruct->ADC_Input));
  assert_param(IS_ADC_CONVERSIONMODE(ADC_InitStruct->ADC_ConversionMode)); 
  assert_param(IS_ADC_ATTENUATION(ADC_InitStruct->ADC_Attenuation));    
  assert_param(IS_ADC_REFERENCEVOLTAGE(ADC_InitStruct->ADC_ReferenceVoltage)); 

  /* Set the decimation rate */
  ADC->CONF_b.DECIM_RATE = ADC_InitStruct->ADC_DecimationRate;
  
  /* Select the input source */
  if(ADC_InitStruct->ADC_Input == ADC_Input_Microphone) {
    ADC->CONF_b.MIC_SEL = SET;
  }
  else {
    ADC->CONF_b.MIC_SEL = RESET;      
    ADC->CONF_b.CHSEL = ADC_InitStruct->ADC_Input;
  }
  
  /* Set the reference voltage */
  ADC->CONF_b.REFSEL = ADC_InitStruct->ADC_ReferenceVoltage;
  
  /* Set the conversion mode */
  if(ADC_InitStruct->ADC_ConversionMode == ADC_ConversionMode_Continuous) {
    ADC->CONF_b.CONT = SET;
  }
  else {
    ADC->CONF_b.CONT = RESET;
  }
  
  /* Set the attenuation */
  ADC->CONF_b.PGASEL = ADC_InitStruct->ADC_Attenuation;

}


/**
* @brief  Enable disable the ADC conversion.
* @param  NewState: functional state @ref FunctionalState
*         This parameter can be: ENABLE or DISABLE.
* @retval None
*/
void ADC_Cmd(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState)); 
  
  if(NewState==ENABLE) {
    if(ADC->CONF_b.MIC_SEL) {
      ADC->CTRL_b.MIC_ON = SET;
    }
    ADC->CTRL_b.SWSTART = SET;
    ADC->CTRL_b.ON = SET;
  }
  else {
    ADC->CTRL_b.ON = RESET;
    ADC->CTRL_b.STOP = SET;
  }
}


/**
* @brief  Enable disable the ADC DMA feature.
* @param  NewState: functional state @ref FunctionalState
*         This parameter can be: ENABLE or DISABLE.
* @retval None
*/
void ADC_DmaCmd(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState)); 
  
  if(NewState==ENABLE) {
    ADC->CTRL_b.DMA_EN = SET;
  }
  else {
    ADC->CTRL_b.DMA_EN = RESET;
  }
}



/**
* @brief  Enable disable the ADC calibration procedure.
* @param  NewState: functional state @ref FunctionalState
*         This parameter can be: ENABLE or DISABLE.
* @retval None
*/
void ADC_Calibration(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState)); 
  
  if(NewState==ENABLE) {
    ADC->CTRL_b.CALEN = SET;
  }
  else {
    ADC->CTRL_b.RSTCALEN = SET;
    ADC->CTRL_b.CALEN = RESET;
  }
}


/**
* @brief  Enable disable the ADC automatic update of the offset.
* @param  NewState: functional state @ref FunctionalState
*         This parameter can be: ENABLE or DISABLE.
* @retval None
*/
void ADC_AutoOffsetUpdate(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState)); 
  
  if(NewState==ENABLE) {
    ADC->CTRL_b.AUTO_OFFSET = SET;
  }
  else {
    ADC->CTRL_b.AUTO_OFFSET = RESET;
  }
}


/**
* @brief  Enable disable the ADC threshold check.
* @param  NewState: functional state @ref FunctionalState
*         This parameter can be: ENABLE or DISABLE.
* @retval None
*/
void ADC_ThresholdCheck(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState)); 
  
  if(NewState==ENABLE) {
    ADC->CTRL_b.ENAB_COMP = SET;
  }
  else {
    ADC->CTRL_b.ENAB_COMP = RESET;
  }
}


/**
* @brief  Configure the ADC threshold.
* @param  ThresholdLow: ADC threshold low value.
*         ThresholdHigh: ADC threshold high value.
* @retval None
*/
void ADC_ThresholdConfig(uint32_t ThresholdLow, uint32_t ThresholdHigh)
{
  
    ADC->THRESHOLD_HI = ThresholdHigh;
    ADC->THRESHOLD_LO = ThresholdLow;
}


/**
* @brief  Configure the ADC conversion mode.
* @param  cConvertionMode: the value can be
*         @arg ADC_ConversionMode_Single single shot ADC conversion
*         @arg ADC_ConversionMode_Continuous continuous ADC conversion
* @retval None
*/
void ADC_ConversionMode(uint8_t ConvertionMode)
{
  /* Check the parameters */
  assert_param(IS_ADC_CONVERSIONMODE(ConvertionMode));
  
  if(ConvertionMode==ADC_ConversionMode_Continuous) {
    ADC->CONF_b.CONT = SET;
  }
  else {
    ADC->CONF_b.CONT = RESET;
  }
}

/**
* @brief  Enable disable the ADC auto round output data.
* @param  NewState: functional state @ref FunctionalState
*         This parameter can be: ENABLE or DISABLE.
* @retval None
*/
void ADC_RoundConvertedData(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState)); 
  
  if(NewState==ENABLE) {
    ADC->CONF_b.ROUND16 = SET;
  }
  else {
    ADC->CONF_b.ROUND16 = RESET;
  }
}

/**
* @brief  Select the frequency of the clock signal for an external MEMS microphone (through IO0)
* @param  Frequency: the value can be 
*         @arg ADC_MIC_800KH Enable the 800 kHz through IO0 for external MEMS microphone
*         @arg ADC_MIC_1600KH Enable the 1.6 MHz through IO0 for external MEMS microphone
* @retval None
*/
void ADC_SelectFrequencyMic(uint8_t Frequency)
{
  /* Check the parameters */
  assert_param(IS_ADC_MIC_FREQ_SEL(Frequency));
  
  /* Check the value of Frequency */
  if(Frequency == ADC_MIC_800KHZ) {
    ADC->CONF_b.DIG_FILT_CLK = RESET;
  }
  else if(Frequency == ADC_MIC_1600KHZ) {
    ADC->CONF_b.DIG_FILT_CLK = SET;
  }

}


/**
* @brief  Get the status flag of ADC.
* @param  ADC_Flag: the value can be
*         @arg ADC_FLAG_CAL ADC End of Calibration flag
*         @arg ADC_FLAG_BUSY ADC busy flag
*         @arg ADC_FLAG_EOC ADC End of Conversion flag
*         @arg ADC_FLAG_WDG ADC ADV value within set threshold 
* @retval FlagStatus: functional state @ref FlagStatus
*         This parameter can be: SET or RESET.
*/
FlagStatus ADC_GetFlagStatus(uint8_t ADC_Flag)
{
  /* Check the parameters */
  assert_param(IS_ADC_GET_FLAG(ADC_Flag));
  
  /* Check the status of the specified SPI flag */
  if (READ_BIT(ADC->SR_REG, ADC_Flag) != (uint16_t)RESET) {
    /* SPI_FLAG is set */
    return SET;
  }
  else {
    /* SPI_FLAG is reset */
    return RESET;
  }
  
}

/**
* @brief  Get the status of the masked IT flag.
*         The read operation will clear the flags.
* @param  ADC_Flag: the value can be 
*         @arg ADC_FLAG_CAL ADC End of Calibration flag
*         @arg ADC_FLAG_BUSY ADC busy flag
*         @arg ADC_FLAG_EOC ADC End of Conversion flag
*         @arg ADC_FLAG_WDG ADC ADV value within set threshold 
* @retval  ITStatus: functional state @ref ITStatus
*         This parameter can be: SET or RESET.
*/
ITStatus ADC_GetITStatus(uint8_t ADC_Flag)
{
  /* Check the parameters */
  assert_param(IS_ADC_GET_FLAG(ADC_Flag));
  
  /* Check the status of the specified SPI interrupt */
  if (READ_BIT(ADC->IRQSTAT, ADC_Flag) != (uint16_t)RESET) {
    /* ADC_Flag is set */
    return SET;
  }
  else {
    /* SPI_IT is reset */
    return RESET;
  }
  
}



/**
* @brief  Enable disable the ADC IT flag.
* @param  ADC_Flag: the value can be 
*         @arg ADC_FLAG_CAL ADC End of Calibration flag
*         @arg ADC_FLAG_BUSY ADC busy flag
*         @arg ADC_FLAG_EOC ADC End of Conversion flag
*         @arg ADC_FLAG_WDG ADC ADV value within set threshold 
* @param  NewState: functional state @ref FunctionalState
*         This parameter can be: ENABLE or DISABLE.
* @retval None
*/
void ADC_ITConfig(uint8_t ADC_Flag, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  assert_param(IS_ADC_GET_FLAG(ADC_Flag));
  
  if (NewState != DISABLE) {
    /* Enable the selected SPI interrupts */
    CLEAR_BIT(ADC->IRQMASK, ADC_Flag);
  }
  else {
    /* Disable the selected SPI interrupts */
    SET_BIT(ADC->IRQMASK, ADC_Flag);
  }
}

/**
* @brief  Get the ADC converted value in Volt.
* @param  dataType: the selected channel, this value can be
*         @arg ADC_Input_AdcPin1 data from ADC pin1
*         @arg ADC_Input_AdcPin2 data from ADC pin2
*         @arg ADC_Input_AdcPin12 data from ADC pin12
*         @arg ADC_Input_Internal0V60V6 data from internal 0.6V
*         @arg ADC_Input_Internal1V20V0 data from internal 1.2V
*         @arg ADC_Input_BattSensor data from battery sensor
*         @arg ADC_Input_TempSensor data from temperature sensor
* @param  vRef: voltage reference configured, the value can be
*         @arg ADC_ReferenceVoltage_0V Vref is 0.0 V
*         @arg ADC_ReferenceVoltage_0V4 Vref is 0.4 V
*         @arg ADC_ReferenceVoltage_0V6 Vref is 0.6 V
*         @arg ADC_ReferenceVoltage_1V2 Vref is 1.2 V
* @retval Converted ADC value in Volt
*/
float ADC_GetConvertedData(uint8_t DataType, uint8_t Vref)
{
  int32_t raw_value;
  uint8_t pga_reg;
  
  /* Check the parameters */
  assert_param(IS_ADC_INPUT(DataType));
  assert_param(IS_ADC_REFERENCEVOLTAGE(Vref)); 

  if(ADC->CONF_b.ROUND16 == SET) {    
    raw_value = (int16_t)(ADC_GetRawData());
  }
  else {
    raw_value = (int32_t)ADC_GetRawData();
  }
  pga_reg = ADC->CONF_b.PGASEL;
  
  if(DataType==ADC_Input_AdcPin1) {
    return ADC_ConvertSingleEndedVoltage(raw_value, (uint8_t)ADC_Input_AdcPin1, Vref, pga_reg);
  }
  else if(DataType==ADC_Input_AdcPin2) {
    return ADC_ConvertSingleEndedVoltage(raw_value, (uint8_t)ADC_Input_AdcPin2, Vref, pga_reg);
  }
  else if(DataType==ADC_Input_AdcPin12 || DataType==ADC_Input_Internal0V60V6 || DataType==ADC_Input_Internal1V20V0) {
    return ADC_ConvertDifferentialVoltage(raw_value, pga_reg);
  }
  else if(DataType==ADC_Input_BattSensor) {
    return ADC_ConvertBatterySensor(raw_value, Vref);
  }
  else if(DataType==ADC_Input_TempSensor) {
    return ADC_ConvertTemperatureSensor(raw_value, Vref, pga_reg);
  }
  return raw_value;
}



/**
* @}
*/ 

/**
* @}
*/ 

/**
* @}
*/ 

/******************* (C) COPYRIGHT 2016 STMicroelectronics *****END OF FILE****/
