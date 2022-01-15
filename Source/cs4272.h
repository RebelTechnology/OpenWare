#ifndef CS4272_H
#define CS4272_H

#include <stdint.h>
#include "device.h"

// Section 8.1 Mode Control
#define CODEC_MODE_CTRL1_REG						(uint8_t)0x01
#define CODEC_MC_FUNC_MODE(x)						(uint8_t)(((x) & 0x03) << 6)
#define CODEC_MC_RATIO_SEL(x)						(uint8_t)(((x) & 0x03) << 4)
#define CODEC_MC_MASTER_SLAVE						(uint8_t)(1<<3)
#define CODEC_MC_SERIAL_FORMAT(x)					(uint8_t)(((x) & 0x07) << 0)

// Section 8.2 DAC Control
#define CODEC_DAC_CTRL						(uint8_t)0x02
#define CODEC_DAC_CTRL_AUTO_MUTE					(uint8_t)0x80
#define CODEC_DAC_CTRL_FILTER_SEL					(uint8_t)0x40
#define CODEC_DAC_CTRL_DE_EMPHASIS(x)					(uint8_t)(((x) & 0x03) << 4)
#define CODEC_DAC_CTRL_VOL_RAMP_UP					(uint8_t)0x08
#define CODEC_DAC_CTRL_VOL_RAMP_DN					(uint8_t)0x04
#define CODEC_DAC_CTRL_INV_POL(x)					(uint8_t)(((x) & 0x03) << 0)
#define CODEC_DAC_CTRL_INV_POL_AB					(uint8_t)(0x03)
#define CODEC_DAC_CTRL_INV_POL_A					(uint8_t)(0x02)
#define CODEC_DAC_CTRL_INV_POL_B					(uint8_t)(0x01)

// Section 8.3 DAC Volume and Mixing
#define CODEC_DAC_VOL_REG						(uint8_t)0x03
#define CODEC_DAC_VOL_SOFT_RAMP(x)					(uint8_t)(((x) & 0x03) << 4)
#define CODEC_DAC_VOL_ATAPI(x)						(uint8_t)(((x) & 0x0F) << 0)
#define CODEC_DAC_VOL_ATAPI_DEFAULT                                     (uint8_t)((1<<3)|(1<<0))
#define CODEC_DAC_VOL_ATAPI_SWAP                                        (uint8_t)((1<<2)|(1<<1))
#define CODEC_DAC_VOL_CHB_CHA						(uint8_t)(1<<6)
#define CODEC_DAC_SOFT_RAMP						(uint8_t)(1<<5)
#define CODEC_DAC_ZERO_CROSS						(uint8_t)(1<<4)

// Section 8.4 DAC Channel A volume
#define CODEC_DAC_CHA_VOL_REG						(uint8_t)0x04
#define CODEC_DAC_CHA_VOL_MUTE						(uint8_t)0x80
#define CODEC_DAC_CHA_VOL_VOLUME(x)					(uint8_t)(((x) & 0x7F) << 0)

// Section 8.5 DAC Channel B volume
#define CODEC_DAC_CHB_VOL_REG						(uint8_t)0x05
#define CODEC_DAC_CHB_VOL_MUTE						(uint8_t)0x80
#define CODEC_DAC_CHB_VOL_VOLUME(x)					(uint8_t)(((x) & 0x7F) << 0)

// Section 8.6 ADC Control
#define CODEC_ADC_CTRL							(uint8_t)0x06
#define CODEC_ADC_CTRL_DITHER						(uint8_t)0x20
#define CODEC_ADC_CTRL_SER_FORMAT					(uint8_t)0x10
#define CODEC_ADC_CTRL_MUTE(x)						(uint8_t)(((x) & 0x03) << 2)
#define CODEC_ADC_CTRL_HPF(x)						(uint8_t)(((x) & 0x03) << 0)

// Section 8.7 Mode Control 2
#define CODEC_MODE_CTRL2_REG						(uint8_t)0x07
#define CODEC_MODE_CTRL2_LOOP						(uint8_t)0x10
#define CODEC_MODE_CTRL2_MUTE_TRACK					(uint8_t)0x08
#define CODEC_MODE_CTRL2_CTRL_FREEZE					(uint8_t)0x04
#define CODEC_MODE_CTRL2_CTRL_PORT_EN					(uint8_t)0x02
#define CODEC_MODE_CTRL2_POWER_DOWN					(uint8_t)0x01

// Section 8.8 Chip ID
#define CODEC_CHIP_ID_REG						(uint8_t)0x08
#define CODEC_CHIP_ID_PART(x)						(uint8_t)(((x) & 0x0F) << 4)
#define CODEC_CHIP_ID_REV(x)						(uint8_t)(((x) & 0x0F) << 0)

#endif
