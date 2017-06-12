#include "stm32f4xx_hal.h"
#include "stm32f4xx.h"

#ifdef __cplusplus
 extern "C" {
#endif

uint16_t readCV_A(void);
uint16_t readCV_B(void);
uint16_t readCV_C(void);
uint16_t readCV_D(void);
void CV_init(ADC_HandleTypeDef*);
	
void RGB_Update(uint16_t, uint16_t, uint16_t);
void RGB_init(TIM_HandleTypeDef*, TIM_HandleTypeDef*, TIM_HandleTypeDef*);

#ifdef __cplusplus
}
#endif

