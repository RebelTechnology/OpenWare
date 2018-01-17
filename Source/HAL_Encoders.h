#ifndef __HAL_ENCODERS_H
#define __HAL_ENCODERS_H


#ifdef __cplusplus
	extern "C" {
#endif

// Prototypes
void Encoders_readAll(void);
void Encoders_readSwitches(void);
void Encoders_init (SPI_HandleTypeDef *spiconfig);

#ifdef __cplusplus
}
#endif
#endif
