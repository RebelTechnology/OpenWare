/**
******************************************************************************
* @file    BlueNRG1_spi.c
* @author  VMA Application Team
* @version V2.0.0
* @date    21-March-2016
* @brief   This file provides all the SPI firmware functions.
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
#include "BlueNRG1_spi.h"

/** @addtogroup BLUENRG1_Peripheral_Driver BLUENRG1 Peripheral Driver
* @{
*/

/** @addtogroup SPI_Peripheral  SPI Peripheral
* @{
*/ 

/** @defgroup SPI_Private_TypesDefinitions Private Types Definitions
* @{
*/

/**
* @}
*/ 

/** @defgroup SPI_Private_Defines Private Defines
* @{
*/

#define MAX_SCR         (255)
#define MAX_CPSVDR      (254)
#define MIN_DIVIDER     (1)
#define MAX_DIVIDER     (65024) // MAX_CPSVDR * (MAX_SCR+1)

#define SPI_CLOCK       (16000000)

/**
* @}
*/

/** @defgroup SPI_Private_Macros Private Macros
* @{
*/

/**
* @}
*/

/** @defgroup SPI_Private_Variables Private Variables
* @{
*/

/**
* @}
*/

/** @defgroup SPI_Private_FunctionPrototypes Private Function Prototypes
* @{
*/

/**
* @}
*/

/** @defgroup SPI_Private_Functions Private Functions
* @{
*/

/**
* @brief  SPI set baud rate function.
* @param  Baudrate: the SPI baud rate up to 8MHz for master mode
*                   and 1MHz for slave mode.
* @retval  None
*/
void SPI_SetBaudrate(uint32_t Baudrate)
{
  uint32_t c;
  uint8_t scr = 1;
  uint8_t cpsr = 2;
  
  /* Check the parameters */
  assert_param(IS_SPI_BAUDRATE(Baudrate));
  
  /* bit rate is: 16MHz / (CPSDVR * (1+SCR))
  and CPSDVR must be an even number from 2-254.
  We calculate (1/2 * CPSDVR) * (1+SCR) */
  c = SPI_CLOCK / (2 * Baudrate);
  
  /* if the bit rate is possible */
  if ((c >= MIN_DIVIDER) && (c <= MAX_DIVIDER)) {
    /* adjust the ratio CPSDVR / (SCR+1) */
    while (c > MAX_SCR) {
      cpsr <<= 1;
      c >>= 1;
    }    
    scr = c - 1;    
  }
  /* set dividers */
  SPI->CPSR = cpsr;
  SPI->CR0_b.SCR = scr;    
}

/**
* @}
*/

/** @defgroup SPI_Public_Functions Public Functions
* @{
*/

/**
* @brief  Deinitializes the SPI peripheral registers to their default
*         reset values.
* @param  None
* @retval None
*/
void SPI_DeInit(void)
{
  SPI->CR0   = 0x1C000000;
  SPI->CR1   = 0x00000000;
  SPI->CPSR  = 0x00000000;
  SPI->IMSC  = 0x00000000;
  SPI->ICR   = 0x00000000;
  SPI->DMACR = 0x00000000;
  SPI->RXFRM = 0x00000000;
  SPI->CHN   = 0x00000000;
  SPI->WDTXF = 0x00000000;
}

/**
* @brief  Initializes the SPI peripheral according to the specified 
*         parameters in the SPI_InitStruct.
* @param  SPI_InitStruct: pointer to a @ref SPI_InitType structure that
*         contains the configuration information for the specified SPI peripheral.
* @retval None
*/
void SPI_Init(SPI_InitType* SPI_InitStruct)
{
  /* check the parameters */
  assert_param(IS_SPI_MODE(SPI_InitStruct->SPI_Mode));
  assert_param(IS_SPI_DATASIZE(SPI_InitStruct->SPI_DataSize));
  assert_param(IS_SPI_CPOL(SPI_InitStruct->SPI_CPOL));
  assert_param(IS_SPI_CPHA(SPI_InitStruct->SPI_CPHA));
  
  /* Set the specified baud rate */
  SPI_SetBaudrate(SPI_InitStruct->SPI_BaudRate);
  
  /* Set SPI mode */
  SPI->CR1_b.MS = SPI_InitStruct->SPI_Mode;
  
  /* Set CPOL */
  SPI->CR0_b.SPO = SPI_InitStruct->SPI_CPOL;
  
  /* Set CPHA */
  SPI->CR0_b.SPH = SPI_InitStruct->SPI_CPHA;
  
  /* Set datasize */
  SPI->CR0_b.DSS = SPI_InitStruct->SPI_DataSize;
}


/**
* @brief  Fills each SPI_InitStruct member with its default value.
* @param  SPI_InitStruct: pointer to a @ref SPI_InitType structure which will be initialized.
* @retval None
*/
void SPI_StructInit(SPI_InitType* SPI_InitStruct)
{
  /* Initialize the SPI_Mode member */
  SPI_InitStruct->SPI_Mode = SPI_Mode_Slave;
  
  /* Initialize the SPI_DataSize member */
  SPI_InitStruct->SPI_DataSize = SPI_DataSize_8b;
  
  /* Initialize the SPI_CPOL member */
  SPI_InitStruct->SPI_CPOL = SPI_CPOL_Low;
  
  /* Initialize the SPI_CPHA member */
  SPI_InitStruct->SPI_CPHA = SPI_CPHA_1Edge;
  
  /* Initialize the SPI_BaudRate member */
  SPI_InitStruct->SPI_BaudRate = 1000000;
}

/**
* @brief  Enables or disables the SPI peripheral.
* @param  NewState: functional state @ref FunctionalState
*         This parameter can be: ENABLE or DISABLE.
* @retval None
*/
void SPI_Cmd(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  
  if (NewState != DISABLE) {
    /* Enable the selected SPI peripheral */
    SPI->CR1_b.SSE = SET;
  }
  else {
    /* Disable the selected SPI peripheral */
    SPI->CR1_b.SSE = RESET;
  }
}

/**
* @brief  Enables or disables the specified SPI interrupts.
* @param  SPI_IT: specifies the SPI interrupt source to be enabled or disabled. 
*         This parameter can be any combination of the following values:
*         @arg SPI_IT_TX: Transmit FIFO half empty or less condition interrupt mask.
*         @arg SPI_IT_RX: Receive FIFO half full or less condition interrupt mask.
*         @arg SPI_IT_RT: Receive FIFO not empty and no read prior to timeout period interrupt mask.
*         @arg SPI_IT_ROR: Receive FIFO written to while full condition interrupt mask.
*         @arg SPI_IT_TUR: Transmit underrun interrupt mask.
*         @arg SPI_IT_TE: Transmit FIFO empty interrupt mask.
* @param  NewState: functional state @ref FunctionalState
*         This parameter can be: ENABLE or DISABLE.
* @retval None
*/
void SPI_ITConfig(uint8_t SPI_IT, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  assert_param(IS_SPI_CONFIG_IT(SPI_IT));
  
  if (NewState != DISABLE) {
    /* Enable the selected SPI interrupts */
    SET_BIT(SPI->IMSC, SPI_IT);
  }
  else {
    /* Disable the selected SPI interrupts */
    CLEAR_BIT(SPI->IMSC, SPI_IT);
  }
}


/**
* @brief  Enables or disables the output if in slave mode.
* @param  NewState: functional state @ref FunctionalState
*         This parameter can be: ENABLE or DISABLE.
* @retval None
*/
void SPI_SlaveModeOutputCmd(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  
  if (NewState != DISABLE) {
    /* Enable the output */
    SPI->CR1_b.SOD = RESET;
  }
  else {
    /* Disable the output */
    SPI->CR1_b.SOD = SET;
  }
}

/**
* @brief  Transmits a Data through the SPI peripheral.
* @param  Data: Data to be transmitted.
* @retval None
*/
void SPI_SendData(uint32_t Data)
{
  /* Write in the DR register the data to be sent */
  SPI->DR = Data;
}

/**
* @brief  Returns the most recent received data by the SPI peripheral. 
* @param  None
* @retval The value of the received data.
*/
uint32_t SPI_ReceiveData(void)
{
  /* Return the data in the DR register */
  return SPI->DR;
}

/**
* @brief  Configures the data size for the selected SPI.
* @param  SPI_DataSize: specifies the SPI data size.
*         This parameter can be one of the following values:
*         @arg SPI_DataSize_32b: Set data frame format to 32bit
*         ...
*         @arg SPI_DataSize_5b: Set data frame format to 5bit
*         @arg SPI_DataSize_4b: Set data frame format to 4bit
* @retval None
*/
void SPI_DataSizeConfig(uint16_t SPI_DataSize)
{
  /* Check the parameters */
  assert_param(IS_SPI_DATASIZE(SPI_DataSize));
  
  /* set the datasize */  
  SPI->CR0_b.DSS = SPI_DataSize;
}

/**
* @brief  Checks whether the specified SPI flag is set or not.
* @param  SPI_FLAG: specifies the SPI flag to check. 
*         This parameter can be any combination of the following values:
*         @arg SPI_IT_TX: Transmit FIFO half empty or less condition interrupt mask.
*         @arg SPI_IT_RX: Receive FIFO half full or less condition interrupt mask.
*         @arg SPI_IT_RT: Receive FIFO not empty and no read prior to timeout period interrupt mask.
*         @arg SPI_IT_ROR: Receive FIFO written to while full condition interrupt mask.
*         @arg SPI_IT_TUR: Transmit underrun interrupt mask.
*         @arg SPI_IT_TE: Transmit FIFO empty interrupt mask.
* @retval FlagStatus: functional state @ref FlagStatus
*         This parameter can be: SET or RESET.
*/
FlagStatus SPI_GetFlagStatus(uint16_t SPI_FLAG)
{
  /* Check the parameters */
  assert_param(IS_SPI_GET_FLAG(SPI_FLAG));
  
  /* Check the status of the specified SPI flag */
  if (READ_BIT(SPI->SR, SPI_FLAG) != (uint16_t)RESET) {
    /* SPI_FLAG is set */
    return SET;
  }
  else {
    /* SPI_FLAG is reset */
    return RESET;
  }
  
}

/**
* @brief  Checks whether the specified SPI interrupt has occurred or not.
* @param  SPI_IT: specifies the SPI interrupt source to check. 
*         This parameter can be one of the following values:
*         @arg SPI_IT_TX: Transmit FIFO half empty or less condition interrupt mask.
*         @arg SPI_IT_RX: Receive FIFO half full or less condition interrupt mask.
*         @arg SPI_IT_RT: Receive FIFO not empty and no read prior to timeout period interrupt mask.
*         @arg SPI_IT_ROR: Receive FIFO written to while full condition interrupt mask.
*         @arg SPI_IT_TUR: Transmit underrun interrupt mask.
*         @arg SPI_IT_TE: Transmit FIFO empty interrupt mask.
* @retval  ITStatus: functional state @ref ITStatus
*         This parameter can be: SET or RESET.
*/
ITStatus SPI_GetITStatus(uint8_t SPI_IT)
{
  /* Check the parameters */
  assert_param(IS_SPI_GET_IT(SPI_IT));
  
  /* Check the status of the specified SPI interrupt */
  if (READ_BIT(SPI->RIS, SPI_IT) != (uint16_t)RESET) {
    /* SPI_IT is set */
    return SET;
  }
  else {
    /* SPI_IT is reset */
    return RESET;
  }
  
}

/**
* @brief  Clears the SPI interrupt pending bits.
* @param  SPI_IT: specifies the SPI interrupt pending bit to clear.
*         This parameter can be one of the following values:
*         @arg SPI_IT_TX: Transmit FIFO half empty or less condition interrupt mask.
*         @arg SPI_IT_RX: Receive FIFO half full or less condition interrupt mask.
*         @arg SPI_IT_RT: Receive FIFO not empty and no read prior to timeout period interrupt mask.
*         @arg SPI_IT_ROR: Receive FIFO written to while full condition interrupt mask.
*         @arg SPI_IT_TUR: Transmit underrun interrupt mask.
*         @arg SPI_IT_TE: Transmit FIFO empty interrupt mask.
* @retval None
*/
void SPI_ClearITPendingBit(uint8_t SPI_IT)
{
  /* Check the parameters */
  assert_param(IS_SPI_CLEAR_IT(SPI_IT));
  
  /* Clear the selected SPI interrupt pending bit */
  CLEAR_BIT(SPI->ICR, SPI_IT);
}

/**
* @brief  Clear all data present within the RX FIFO of the SPI peripherial.
*					SPI peripherial clocks have to be enabled first before the use.
* @param  None
* @retval None
*/
void SPI_ClearRXFIFO(void)
{
  /* Flush the data from SPI RX FIFO */
  while(SET == SPI_GetFlagStatus(SPI_FLAG_RNE) ) {
    SPI_ReceiveData();
  }	
}

/**
* @brief  Clear all data present within the TX FIFO of the SPI peripherial.
*					SPI peripherial clocks have to be enabled first before the use.
* @param  None
* @retval None
*/
void SPI_ClearTXFIFO(void)
{
  uint32_t tmp;

  /* Enable read from the TX FIFO */
  SPI->ITCR_b.SWAPFIFO = SET;
  
  /* Flush the data from the TX FIFO till it is not empty. */
  while(0 == SPI->SR_b.TFE) {
    tmp |= SPI->TDR;
  }
  
  /* Disable the read from the TX FIFO */
  SPI->ITCR_b.SWAPFIFO = RESET;

}

/**
* @brief Set the SPI communication mode.
* @param  Mode: specifies the SPI communication mode.
*         This parameter can be one of the following values:
*         @arg SPI_FULL_DUPLEX_MODE:  SPI full duplex communication mode.
*         @arg SPI_TRANSMIT_MODE: SPI transmit communication mode.
*         @arg SPI_RECEIVE_MODE: SPI receive communication mode.
*					@arg SPI_COMBINED_MODE: SPI combined communication mode.
* @retval None
*/
void SPI_SetMasterCommunicationMode(uint32_t Mode)
{
  /* Check the parameters */
  assert_param(IS_SPI_COM_MODE(Mode));
  
  /* Set the communication mode */
  SPI->CR0_b.SPIM = Mode;
}

/**
* @brief Set the dummy character used for the SPI master communication.
* @param  NullCharacter: Dummy character to be used.
* @retval None
*/
void SPI_SetDummyCharacter(uint32_t NullCharacter)
{
  /* Set the dummy character */
  SPI->CHN = NullCharacter;
}

/**
* @brief Set the number of frames to receive from slave.
* @param  Number: Number of frames to receive.
* @retval None
*/
void SPI_SetNumFramesToReceive(uint16_t Number)
{
  /* Set the number of frames to receive */
  SPI->RXFRM = Number;
}

/**
* @brief Set the number of frames to transmit to slave
* @param  Number: Number of frames to transmit.
* @retval None
*/
void SPI_SetNumFramesToTransmit(uint16_t Number)
{
  /* Set the number of frames to transmit */
  SPI->WDTXF = Number;
}


/**
* @brief  Master can select the slave by driving the CS pin by software.
* @param  NewState: functional state @ref FunctionalState
*         This parameter can be: ENABLE or DISABLE.
* @retval None
*/
void SPI_SlaveSwSelection(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  
  if (NewState != DISABLE) {
    /* Enable the output */
    SPI->CR0_b.CS1 = RESET;
  }
  else {
    /* Disable the output */
    SPI->CR0_b.CS1 = SET;
  }
}

/**
* @brief  Master can select the slave by driving the CS pin by software.
* @param  Endian: the endian format @ref SPI_Endian_Format_Definition
* @retval None
*/
void SPI_EndianFormatReception(uint8_t Endian)
{
  /* Check the parameters */
  assert_param(IS_SPI_ENDIAN(Endian));
  
  SPI->CR1_b.RENDN = Endian;
}

/**
* @brief  Master can select the slave by driving the CS pin by software.
* @param  Endian: the endian format @ref SPI_Endian_Format_Definition
* @retval None
*/
void SPI_EndianFormatTransmission(uint8_t Endian)
{
  /* Check the parameters */
  assert_param(IS_SPI_ENDIAN(Endian));
  
  SPI->CR1_b.TENDN = Endian;
}


/**
* @brief  Configure the delay between the frames.
*         The MSPIWAIT value is used to insert a wait state between frames.
*         The SSPFRM pulse duration is given by the equation:
*         SSPCLKOUT x {MSPIWAIT - (SCR-1) / [2x(SCR+1)]}.
*         When SCR=1, MSPIWAIT indicates the number of SSPCLKO cycle that
*         the SSPFRM pulse duration should have between frames. 
* @param  Delay: this value must be less than or equal to 15.
*         0 means no delay (default).
* @retval None
*/
void SPI_DelayBetweenFrames(uint8_t Delay)
{
  /* Check the parameters */
  assert_param(IS_SPI_ENDIAN(Delay));
  
  SPI->CR1_b.MSPIWAIT = Delay;
}


/**
* @brief  Master insert a delay of two clock cycle if enabled.
* @param  NewState: functional state @ref FunctionalState
*         This parameter can be: ENABLE or DISABLE.
* @retval None
*/
void SPI_DelayDataInput(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  
  if (NewState != DISABLE) {
    /* Enable the output */
    SPI->CR1_b.DATAINDEL = RESET;
  }
  else {
    /* Disable the output */
    SPI->CR1_b.DATAINDEL = SET;
  }
}


/**
  * @brief  Sets the SPI interrupt FIFO level.
  * @param  SPI_TX_FIFO_LEV: specifies the transmit interrupt FIFO level.
  *   This parameter can be one of the following values:
  *   @arg FIFO_TX_LEV_1: interrupt when TX FIFO contains 1 element or more
  *   @arg FIFO_TX_LEV_4: interrupt when TX FIFO contains 4 elements or more
  *   @arg FIFO_TX_LEV_8: interrupt when TX FIFO contains 8 elements or more
  * @retval None
  */
void SPI_TxFifoInterruptLevelConfig(uint8_t SPI_TX_FIFO_LEV)
{
    /* Check the parameters */
  assert_param(IS_SPI_FIFO_LEV(SPI_TX_FIFO_LEV));

  SPI->CR1_b.TXIFLSEL = SPI_TX_FIFO_LEV;
}

/**
  * @brief  Sets the SPI interrupt FIFO level.
  * @param  SPI_RX_FIFO_LEV: specifies the receive interrupt FIFO level.
  *   This parameter can be one of the following values:
  *   @arg FIFO_RX_LEV_1: interrupt when RX FIFO contains 1 element or more
  *   @arg FIFO_RX_LEV_4: interrupt when RX FIFO contains 4 elements or more
  *   @arg FIFO_RX_LEV_8: interrupt when RX FIFO contains 8 elements or more
  *   
  * @retval None
  */
void SPI_RxFifoInterruptLevelConfig(uint8_t SPI_RX_FIFO_LEV)
{
    /* Check the parameters */
  assert_param(IS_SPI_FIFO_LEV(SPI_RX_FIFO_LEV));

  SPI->CR1_b.RXIFLSEL = SPI_RX_FIFO_LEV;
}

/**
  * @brief  Enables or disables the SPI DMA interface.
  * @param  SPI_DMAReq: specifies the DMA request.
  *   This parameter can be any combination of the following values:
  *     @arg SPI_DMAReq_Tx: SPI DMA transmit request
  *     @arg SPI_DMAReq_Rx: SPI DMA receive request.
  * @param  NewState: functional state @ref FunctionalState
  *   This parameter can be: ENABLE or DISABLE.   
  * @retval None
  */
void SPI_DMACmd(uint8_t SPI_DMAReq, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_SPI_DMAREQ(SPI_DMAReq));  
  assert_param(IS_FUNCTIONAL_STATE(NewState)); 

  if (NewState != DISABLE)
  {
    /* Enable the DMA transfer */
    SPI->DMACR |= SPI_DMAReq;
  }
  else
  {
    /* Disable the DMA transfer */
    SPI->DMACR &= (uint8_t)~SPI_DMAReq;
  }
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
