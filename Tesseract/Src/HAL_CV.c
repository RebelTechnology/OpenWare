#ifndef HAL_IO_H
#define HAL_IO_H

#include "HAL_CV.h"

#ifdef __cplusplus
 extern "C" {
#endif
	 
ADC_HandleTypeDef* hadc_3;
	 
TIM_HandleTypeDef* htim_R;
TIM_HandleTypeDef* htim_G;
TIM_HandleTypeDef* htim_B;
	 
uint16_t rgADC_Result[8];

uint16_t readCV_A(void)
{
	// ADC 1
	return (rgADC_Result[0]+rgADC_Result[4])/2;
}

uint16_t readCV_B(void)
{
	// ADC 2
	return (rgADC_Result[1]+rgADC_Result[5])/2;
}

uint16_t readCV_C(void)
{
	// ADC 3
	return (rgADC_Result[2]+rgADC_Result[6])/2;
}

uint16_t readCV_D(void)
{
	// ADC 4
	return (rgADC_Result[3]+rgADC_Result[7])/2;
}
	 
// CV Control
void CV_init(ADC_HandleTypeDef* hadc)
{
	hadc_3 = hadc;
	
	// Start ADC Conversions
	/* HAL_ADC_Start(hadc_3); */
	HAL_ADC_Start_DMA(hadc_3, (uint32_t*)rgADC_Result, 8);
	
}
	 
// RGB LED Control
void RGB_init(TIM_HandleTypeDef* htim1, TIM_HandleTypeDef* htim2, TIM_HandleTypeDef* htim3)
{
	htim_R = htim1;
	htim_G = htim2;
	htim_B = htim3;
	
	// Red
	HAL_TIM_Base_Start(htim_R);
  HAL_TIM_PWM_Start(htim_R, TIM_CHANNEL_1);
  HAL_TIMEx_PWMN_Start(htim_R,TIM_CHANNEL_1);
	
	// Green
	HAL_TIM_Base_Start(htim_G);
  HAL_TIM_PWM_Start(htim_G, TIM_CHANNEL_4);
  HAL_TIMEx_PWMN_Start(htim_G,TIM_CHANNEL_4);

	// Blue
	HAL_TIM_Base_Start(htim_B);
  HAL_TIM_PWM_Start(htim_B, TIM_CHANNEL_2);
  HAL_TIMEx_PWMN_Start(htim_B,TIM_CHANNEL_2);
}

void RGB_Update(uint16_t red, uint16_t green, uint16_t blue)
{
	__HAL_TIM_SET_COMPARE(htim_R, TIM_CHANNEL_1, red);
	__HAL_TIM_SET_COMPARE(htim_G, TIM_CHANNEL_4, green);
	__HAL_TIM_SET_COMPARE(htim_B, TIM_CHANNEL_2, blue);
}


#endif
