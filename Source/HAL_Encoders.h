#ifndef __HAL_ENCODERS_H
#define __HAL_ENCODERS_H

#include <stdint.h>
#include "device.h"

#ifdef __cplusplus
	extern "C" {
#endif

// Prototypes
void Encoders_readAll(void);
void Encoders_readSwitches(void);
void Encoders_init (SPI_HandleTypeDef *spiconfig);
int16_t* Encoders_get();

#ifdef __cplusplus
}
#endif
#endif
