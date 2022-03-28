#ifndef SDRAM_H
#define SDRAM_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "main.h"
#include "errorhandlers.h"

#ifdef USE_EXTERNAL_RAM
void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram);
#endif

void MPU_Config(void);

#ifdef __cplusplus
}
#endif

#endif /* SDRAM_H */

