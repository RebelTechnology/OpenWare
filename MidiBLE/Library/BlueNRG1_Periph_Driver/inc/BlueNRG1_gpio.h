/**
  ******************************************************************************
  * @file    BlueNRG1_gpio.h
  * @author  VMA Application Team
  * @version V2.0.0
  * @date    21-March-2016
  * @brief   This file contains all the functions prototypes for the GPIO 
  *          firmware library.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef BLUENRG1_GPIO_H
#define BLUENRG1_GPIO_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "BlueNRG1.h"

/** @addtogroup BLUENRG1_Peripheral_Driver BLUENRG1 Peripheral Driver
  * @{
  */

/** @addtogroup GPIO_Peripheral  GPIO Peripheral
  * @{
  */


/** @defgroup GPIO_Exported_Types Exported Types
  * @{
  */

/** 
  * @brief  Structure definition of GPIO initialization
  */
typedef struct
{
  uint16_t GPIO_Pin;              /*!< Specifies the GPIO pins to be configured.
                                      This parameter can be any value of @ref GPIO_Pins_Definition */
  
  uint8_t GPIO_Mode;             /*!< Specifies the operating mode for the selected pin.
                                      This parameter can be a value of @ref GPIO_Mode_Definition */

  FunctionalState GPIO_HighPwr;   /*!< Specifies the GPIO strength for the selected pin.
                                      This parameter can be any value of @ref FunctionalState */
  
  FunctionalState GPIO_Pull;          /*!< Specifies the GPIO pull down state for the selected pin.
                                      This parameter can be a value of @ref FunctionalState */
} GPIO_InitType;



/**
  * @brief  Structure definition of GPIO EXTernal Interrupt initialization
  */
typedef struct
{
  uint32_t GPIO_Pin;              /*!< Specifies the GPIO pins to be configured.
                                      This parameter can be any value of @ref GPIO_Pins_Definition */
  
  uint32_t GPIO_IrqSense;         /*!< Specifies the GPIO interrupt sense to be configured.
                                      This parameter can be any value of @ref GPIO_IRQ_Sense_Definition */
  
  uint32_t GPIO_Event;            /*!< Specifies the GPIO level sense to be configured.
                                      This parameter can be any value of @ref GPIO_Event_Definition */
  
} GPIO_EXTIConfigType;


/** 
  * @brief  Bit_SET and Bit_RESET enumeration  
  */
typedef enum
{
  Bit_RESET = 0,  /*!< Specifies the GPIO reset state */
  Bit_SET         /*!< Specifies the GPIO set state */
} BitAction;

/** This macro checks if ACTION is a valid bit action */
#define IS_GPIO_BIT_ACTION(ACTION) (((ACTION) == Bit_RESET) || ((ACTION) == Bit_SET))

/**
  * @}
  */


/** @defgroup GPIO_Exported_Constants Exported Constants
  * @{
  */


	
/** @defgroup GPIO_Mode_Definition Mode Definition
  * @{
  */  

#define GPIO_Input                  ((uint8_t)0)  /*!< GPIO input selected */
#define Serial1_Mode                ((uint8_t)1)  /*!< Serial1 mode selected */
#define Serial0_Mode                ((uint8_t)4)  /*!< Serial0 mode selected */
#define Microphone_Mode             ((uint8_t)5)  /*!< Microphone mode selected */
#define GPIO_Output                 ((uint8_t)6)  /*!< GPIO output mode selected */

/* This macro checks if MODE is a valid mode value for GPIO, please refer to the DATASHEET GPIO mode table */
#define IS_GPIO_MODE(MODE) (((MODE) == GPIO_Input) || \
                            ((MODE) == GPIO_Output) || \
                            ((MODE) == Serial1_Mode) || \
                            ((MODE) == Serial0_Mode) || \
                            ((MODE) == Microphone_Mode))

/**
  * @}
  */



/** @defgroup GPIO_Pins_Definition GPIO Pins Definition
  * @{
  */
#define GPIO_Pin_0                 ((uint16_t)0x0001)  /*!< Pin 0 selected */
#define GPIO_Pin_1                 ((uint16_t)0x0002)  /*!< Pin 1 selected */
#define GPIO_Pin_2                 ((uint16_t)0x0004)  /*!< Pin 2 selected */
#define GPIO_Pin_3                 ((uint16_t)0x0008)  /*!< Pin 3 selected */
#define GPIO_Pin_4                 ((uint16_t)0x0010)  /*!< Pin 4 selected */
#define GPIO_Pin_5                 ((uint16_t)0x0020)  /*!< Pin 5 selected */
#define GPIO_Pin_6                 ((uint16_t)0x0040)  /*!< Pin 6 selected */
#define GPIO_Pin_7                 ((uint16_t)0x0080)  /*!< Pin 7 selected */
#define GPIO_Pin_8                 ((uint16_t)0x0100)  /*!< Pin 8 selected */
#define GPIO_Pin_9                 ((uint16_t)0x0200)  /*!< Pin 9 selected */
#define GPIO_Pin_10                ((uint16_t)0x0400)  /*!< Pin 10 selected */
#define GPIO_Pin_11                ((uint16_t)0x0800)  /*!< Pin 11 selected */
#define GPIO_Pin_12                ((uint16_t)0x1000)  /*!< Pin 12 selected */
#define GPIO_Pin_13                ((uint16_t)0x2000)  /*!< Pin 13 selected */
#define GPIO_Pin_14                ((uint16_t)0x4000)  /*!< Pin 14 selected */
#define GPIO_Pin_All               ((uint16_t)0x7FFF)  /*!< All pins selected */

/* Number of total available GPIO pins */
#define GPIO_PIN_COUNT               (15)

#define IS_GPIO_PIN(PIN) (((PIN) == GPIO_Pin_0) || ((PIN) == GPIO_Pin_1) || \
                            ((PIN) == GPIO_Pin_2) || ((PIN) == GPIO_Pin_3) || \
                            ((PIN) == GPIO_Pin_4) || ((PIN) == GPIO_Pin_5) || \
                            ((PIN) == GPIO_Pin_6) || ((PIN) == GPIO_Pin_7) || \
                            ((PIN) == GPIO_Pin_8) || ((PIN) == GPIO_Pin_9) || \
                            ((PIN) == GPIO_Pin_10) || ((PIN) == GPIO_Pin_11) || \
                            ((PIN) == GPIO_Pin_12) || ((PIN) == GPIO_Pin_13) || \
                            ((PIN) == GPIO_Pin_14))


/* This macro checks if PIN is a valid pin combination value */
#define IS_GPIO_PINS(PIN) ((PIN & GPIO_Pin_All) != 0)


/**
  * @}
  */


/** @defgroup GPIO_IRQ_Sense_Definition IRQ Sense Definition
  * @{
  */

#define GPIO_IrqSense_Edge      (0)   /*!< Edge detection */
#define GPIO_IrqSense_Level     (1)   /*!< Level detection */

/** This macro checks if IS is a valid interrupt sense value */
#define IS_GPIO_IRQSENSE(IS) (((IS) == GPIO_IrqSense_Edge) || ((IS) == GPIO_IrqSense_Level))

/**
  * @}
  */


/** @defgroup GPIO_Event_Definition Event	Definition
  * @{
  */
#define GPIO_Event_Low           (0)   /*!< Low level or falling edge detection */
#define GPIO_Event_High          (1)   /*!< High level or rising edge detection */
#define GPIO_Event_Both          (2)   /*!< Both edges detection */

/** This macro checks if EV is a valid GPIO_Event value */
#define IS_GPIO_EVENT(EV) (((EV) == GPIO_Event_Low) || \
                           ((EV) == GPIO_Event_High) || \
                           ((EV) == GPIO_Event_Both))

/**
  * @}
  */  

/**
  * @}
  */

/** @defgroup GPIO_Exported_Macros Exported Macros
* @{
*/

/* 
 * GPIO Configuration
 */

/* GPIO Input Signal */
#define GPIO_InitInputPinx(PIN_NAME)    GPIO_Init(&(GPIO_InitType){PIN_NAME, GPIO_Input, DISABLE, DISABLE});

/* GPIO Output Signal */
#define GPIO_InitOutputPinx(PIN_NAME)   GPIO_Init(&(GPIO_InitType){PIN_NAME, GPIO_Output, DISABLE, DISABLE});


/* 
 * UART Port Configuration
 */

/* UART RX Signal */
#define GPIO_InitUartRxPin4()    GPIO_Init(&(GPIO_InitType){GPIO_Pin_4, Serial1_Mode, DISABLE, DISABLE});
#define GPIO_InitUartRxPin11()   GPIO_Init(&(GPIO_InitType){GPIO_Pin_11, Serial1_Mode, DISABLE, DISABLE});

/* UART TX Signal */
#define GPIO_InitUartTxPin5()    GPIO_Init(&(GPIO_InitType){GPIO_Pin_5, Serial1_Mode, DISABLE, DISABLE});
#define GPIO_InitUartTxPin8()    GPIO_Init(&(GPIO_InitType){GPIO_Pin_8, Serial1_Mode, DISABLE, DISABLE});

/* UART CTS Signal */
#define GPIO_InitUartCtsPin0()    GPIO_Init(&(GPIO_InitType){GPIO_Pin_0, Serial1_Mode, DISABLE, DISABLE});
#define GPIO_InitUartCtsPin7()    GPIO_Init(&(GPIO_InitType){GPIO_Pin_7, Serial1_Mode, DISABLE, DISABLE});
#define GPIO_InitUartCtsPin13()   GPIO_Init(&(GPIO_InitType){GPIO_Pin_13, Serial1_Mode, DISABLE, DISABLE});

/* UART RTS Signal */
#define GPIO_InitUartRtsPin1()    GPIO_Init(&(GPIO_InitType){GPIO_Pin_1, Serial1_Mode, DISABLE, DISABLE});
#define GPIO_InitUartRtsPin6()    GPIO_Init(&(GPIO_InitType){GPIO_Pin_6, Serial1_Mode, DISABLE, DISABLE});


/* 
 * SPI Port Configuration
 */

/* SPI Clock Signal */
#define GPIO_InitSpiClkPin0()    GPIO_Init(&(GPIO_InitType){GPIO_Pin_0, Serial0_Mode, DISABLE, DISABLE});
#define GPIO_InitSpiClkPin8()    GPIO_Init(&(GPIO_InitType){GPIO_Pin_8, Serial0_Mode, DISABLE, DISABLE});
#define GPIO_InitSpiClkPin14()   GPIO_Init(&(GPIO_InitType){GPIO_Pin_14, Serial0_Mode, DISABLE, DISABLE});

/* SPI CS Signal */
#define GPIO_InitSpiCsPin1()     GPIO_Init(&(GPIO_InitType){GPIO_Pin_1, Serial0_Mode, DISABLE, DISABLE});
#define GPIO_InitSpiCsPin11()    GPIO_Init(&(GPIO_InitType){GPIO_Pin_11, Serial0_Mode, DISABLE, DISABLE});

/* SPI MOSI Signal */
#define GPIO_InitSpiMosiPin2()   GPIO_Init(&(GPIO_InitType){GPIO_Pin_2, Serial0_Mode, DISABLE, DISABLE});
#define GPIO_InitSpiMosiPin10()  GPIO_Init(&(GPIO_InitType){GPIO_Pin_10, Serial0_Mode, DISABLE, DISABLE});

/* SPI MISO Signal */
#define GPIO_InitSpiMisoPin9()   GPIO_Init(&(GPIO_InitType){GPIO_Pin_9, Serial0_Mode, DISABLE, DISABLE});
#define GPIO_InitSpiMisoPin3()   GPIO_Init(&(GPIO_InitType){GPIO_Pin_3, Serial0_Mode, DISABLE, DISABLE});


/* 
 * I2C1 Port Configuration
 */

/* I2C1 Clock Signal */
#define GPIO_InitI2c1ClkPin12()  GPIO_Init(&(GPIO_InitType){GPIO_Pin_12, Serial0_Mode, DISABLE, DISABLE});

/* I2C1 Data Signal */
#define GPIO_InitI2c1DataPin13() GPIO_Init(&(GPIO_InitType){GPIO_Pin_13, Serial0_Mode, DISABLE, DISABLE});
#define GPIO_InitI2c1ClkPin14()  GPIO_Init(&(GPIO_InitType){GPIO_Pin_14, Serial1_Mode, DISABLE, DISABLE});


/* 
 * I2C2 Port Configuration
 */

/* I2C2 Clock Signal */
#define GPIO_InitI2c2ClkPin4()   GPIO_Init(&(GPIO_InitType){GPIO_Pin_4, Serial0_Mode, DISABLE, DISABLE});
#define GPIO_InitI2c2ClkPin6()   GPIO_Init(&(GPIO_InitType){GPIO_Pin_6, Serial0_Mode, DISABLE, DISABLE});

/* I2C2 Data Signal */
#define GPIO_InitI2c2DataPin5()  GPIO_Init(&(GPIO_InitType){GPIO_Pin_5, Serial0_Mode, DISABLE, DISABLE});
#define GPIO_InitI2c2DataPin7()  GPIO_Init(&(GPIO_InitType){GPIO_Pin_7, Serial0_Mode, DISABLE, DISABLE});


/* 
 * PWM Configuration
 */

/* PWM0 Signal */
#define GPIO_InitPwm0Pin2()    GPIO_Init(&(GPIO_InitType){GPIO_Pin_2, Serial1_Mode, DISABLE, DISABLE});
#define GPIO_InitPwm0Pin4()    GPIO_Init(&(GPIO_InitType){GPIO_Pin_4, Microphone_Mode, DISABLE, DISABLE});

/* PWM1 Signal */
#define GPIO_InitPwm1Pin3()    GPIO_Init(&(GPIO_InitType){GPIO_Pin_3, Serial1_Mode, DISABLE, DISABLE});
#define GPIO_InitPwm1Pin5()    GPIO_Init(&(GPIO_InitType){GPIO_Pin_5, Microphone_Mode, DISABLE, DISABLE});


/* 
 * PDM Configuration
 */

/* PDM Data Signal */
#define GPIO_InitPdmDataPin1()    GPIO_Init(&(GPIO_InitType){GPIO_Pin_1, Microphone_Mode, DISABLE, DISABLE});
#define GPIO_InitPdmDataPin6()    GPIO_Init(&(GPIO_InitType){GPIO_Pin_6, Microphone_Mode, DISABLE, DISABLE});
#define GPIO_InitPdmDataPin8()    GPIO_Init(&(GPIO_InitType){GPIO_Pin_8, Microphone_Mode, DISABLE, DISABLE});
#define GPIO_InitPdmDataPin14()    GPIO_Init(&(GPIO_InitType){GPIO_Pin_14, Microphone_Mode, DISABLE, DISABLE});

/* PDM Clock Signal */
#define GPIO_InitPdmClockPin2()   GPIO_Init(&(GPIO_InitType){GPIO_Pin_2, Microphone_Mode, DISABLE, DISABLE});
#define GPIO_InitPdmClockPin3()   GPIO_Init(&(GPIO_InitType){GPIO_Pin_3, Microphone_Mode, DISABLE, DISABLE});
#define GPIO_InitPdmClockPin7()   GPIO_Init(&(GPIO_InitType){GPIO_Pin_7, Microphone_Mode, DISABLE, DISABLE});


/**
  * @}
  */


/** @defgroup GPIO_Exported_Functions Exported Functions
  * @{
  */

/* Initialization */
void GPIO_DeInit(void);																					/*!< GPIO deinitialization. */
void GPIO_Init(GPIO_InitType* GPIO_InitStruct);									/*!< GPIO initialization. */
void GPIO_StructInit(GPIO_InitType* GPIO_InitStruct);						/*!< Initialization of the GPIO_InitType Structure. */

/* Data read */
BitAction GPIO_ReadBit(uint16_t GPIO_Pins);												/*!< Read GPIO pin logic state. */

/* Data write */
void GPIO_WriteBit(uint16_t GPIO_Pins, BitAction BitVal);				/*!< Write new GPIO pin logic state. */

/* Bit operations */
void GPIO_SetBits(uint16_t GPIO_Pins);														/*!< Set selected GPIO pin. */
void GPIO_ResetBits(uint16_t GPIO_Pins);													/*!< Reset selected GPIO pin. */
void GPIO_ToggleBits(uint16_t GPIO_Pins);													/*!< Toggle selected GPIO pin. */

/* GPIO interrupts */
void GPIO_EXTIStructInit(GPIO_EXTIConfigType* GPIO_EXTIInitStruct); /*!< Initialization of the GPIO_EXTIInit Structure. */
void GPIO_EXTIConfig(GPIO_EXTIConfigType* EXTIConfig);					/*!< Selects the GPIO pin used as EXTI Line. */
void GPIO_EXTICmd(uint16_t GPIO_Pins, FunctionalState NewState);	/*!< Enables or disables interrupts on specified pins. */
void GPIO_ClearITPendingBit(uint16_t GPIO_Pins);									/*!< Clears the GPIOx interrupt pending bits. */
FlagStatus GPIO_GetITPendingBit(uint16_t GPIO_Pin);								/*!< Checks whether the specified enabled GPIO interrupt is active. */


/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* BLUENRG1_GPIO_H */

/******************* (C) COPYRIGHT 2016 STMicroelectronics *****END OF FILE****/
