
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "HAL_CV.h"

static void Fill_Buffer(uint32_t *pBuffer, uint32_t uwBufferLenght, uint32_t uwOffset);

uint8_t SDRAM_Test(void);

#define BUFFER_SIZE         ((uint32_t)0x000A)
#define WRITE_READ_ADDR     ((uint32_t)0x0800)
#define SDRAM_BANK_ADDR     ((uint32_t)0xD0000000)

/* Read/Write Buffers */
uint32_t aTxBuffer[BUFFER_SIZE];
uint32_t aRxBuffer[BUFFER_SIZE];

void OWLtest(void)
{
	SDRAM_Test();
	RGB_Update(500, 500, 500);
}


uint8_t SDRAM_Test(void)
{
	/* Status variables */
	__IO uint32_t uwWriteReadStatus = 0;
	uint32_t uwIndex = 0;
	
	/*##-2- SDRAM memory read/write access #####################################*/  
			
	/* Fill the buffer to write */
	Fill_Buffer(aTxBuffer, BUFFER_SIZE, 0xA244250F);   

	/* Write data to the SDRAM memory */
	for (uwIndex = 0; uwIndex < BUFFER_SIZE; uwIndex++)
	{
		*(__IO uint32_t*) (SDRAM_BANK_ADDR + WRITE_READ_ADDR + 4*uwIndex) = aTxBuffer[uwIndex];
	}    

	/* Read back data from the SDRAM memory */
	for (uwIndex = 0; uwIndex < BUFFER_SIZE; uwIndex++)
	{
		aRxBuffer[uwIndex] = *(__IO uint32_t*) (SDRAM_BANK_ADDR + WRITE_READ_ADDR + 4*uwIndex);
	} 

	/*##-3- Checking data integrity ############################################*/    

	for (uwIndex = 0; (uwIndex < BUFFER_SIZE) && (uwWriteReadStatus == 0); uwIndex++)
	{
		if (aRxBuffer[uwIndex] != aTxBuffer[uwIndex])
		{
			uwWriteReadStatus++;
		}
	}	

	if (uwWriteReadStatus)
	{
		/* KO */ 
	}
	else
	{ 
		/* OK */
	}

	return (!uwWriteReadStatus);
}

static void Fill_Buffer(uint32_t *pBuffer, uint32_t uwBufferLenght, uint32_t uwOffset)
{
  uint32_t tmpIndex = 0;

  /* Put in global buffer different values */
  for (tmpIndex = 0; tmpIndex < uwBufferLenght; tmpIndex++ )
  {
    pBuffer[tmpIndex] = tmpIndex + uwOffset;
  }
}   
