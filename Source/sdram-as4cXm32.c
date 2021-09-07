#include <stm32h7xx_hal.h>
#include "sdram.h"

/**
 * Alliance Memory DRAM 256Mbits, 3.3V, 8Mx32 AS4C8M32SA-6BIN
 */

#define SDRAM_MODEREG_BURST_LENGTH_2               (1 << 0)
#define SDRAM_MODEREG_BURST_LENGTH_4               (1 << 1)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL        (0 << 3)
#define SDRAM_MODEREG_CAS_LATENCY_3                ((1 << 4) | (1 << 5))
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE       (1 << 9)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROG_BURST   (0 << 9)

void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram){
  FMC_SDRAM_CommandTypeDef Command;

  __IO uint32_t tmpmrd = 0;
  /* Step 3:  Configure a clock configuration enable command */
  Command.CommandMode            = FMC_SDRAM_CMD_CLK_ENABLE;
  Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
  Command.AutoRefreshNumber      = 1;
  Command.ModeRegisterDefinition = 0;

  /* Send the command */
  HAL_SDRAM_SendCommand(hsdram, &Command, 0x1000);

  /* Step 4: Insert 100 ms delay */
  HAL_Delay(100);

  /* Step 5: Configure a PALL (precharge all) command */
  Command.CommandMode            = FMC_SDRAM_CMD_PALL;
  Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
  Command.AutoRefreshNumber      = 1;
  Command.ModeRegisterDefinition = 0;

  /* Send the command */
  HAL_SDRAM_SendCommand(hsdram, &Command, 0x1000);

  /* Step 6 : Configure a Auto-Refresh command */
  Command.CommandMode            = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
  Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
  Command.AutoRefreshNumber      = 4;
  Command.ModeRegisterDefinition = 0;

  /* Send the command */
  HAL_SDRAM_SendCommand(hsdram, &Command, 0x1000);

  /* Step 7: Program the external memory mode register */
  tmpmrd = SDRAM_MODEREG_BURST_LENGTH_4
         | SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL
         | SDRAM_MODEREG_CAS_LATENCY_3
         | SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;  
  Command.CommandMode            = FMC_SDRAM_CMD_LOAD_MODE;
  Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
  Command.AutoRefreshNumber      = 1;
  Command.ModeRegisterDefinition = tmpmrd;

  /* Send the command */
  HAL_SDRAM_SendCommand(hsdram, &Command, 0x1000);

  /* Set refresh rate */
  HAL_SDRAM_ProgramRefreshRate(hsdram, 0x81A - 20);
}
