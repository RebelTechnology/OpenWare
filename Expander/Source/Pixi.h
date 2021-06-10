/*
  Pixi.h - Library for Analog shield with Maxim PIXI A/D chip.
  Created by Wolfgang Friedrich, July 29. 2014.
  Will be released into the public domain.
*/

#ifndef PIXI_H
#define PIXI_H

#include <stdint.h>

//#include "WProgram.h"

//#define PIXI_

// R/WB bit
#define PIXI_READ                  0x01
#define PIXI_WRITE                 0x00
// Register Table (each register is 16 bit wide)
#define PIXI_DEVICE_ID                  0x00
#define PIXI_INTERRUPT                  0x01
#define PIXI_ADC_DATA_STATUS_0_15       0x02
#define PIXI_ADC_DATA_STATUS_16_19      0x03
#define PIXI_OVERCURRENT_STATUS_0_15    0x04
#define PIXI_OVERCURRENT_STATUS_16_19   0x05
#define PIXI_GPI_STATUS_0_15            0x06
#define PIXI_GPI_STATUS_16_19           0x07
#define PIXI_INT_TEMP_DATA              0x08
#define PIXI_EXT1_TEMP_DATA             0x09
#define PIXI_EXT2_TEMP_DATA             0x0A
#define PIXI_GPI_DATA_0_15              0x0B
#define PIXI_GPI_DATA_16_19             0x0C
#define PIXI_GPO_DATA_0_15              0x0D
#define PIXI_GPO_DATA_16_19             0x0E
#define PIXI_DEVICE_CTRL                0x10
#define PIXI_INTERRUPT_MASK             0x11
#define PIXI_GPI_IRQ_MODE_0_7           0x12
#define PIXI_GPI_IRQ_MODE_8_15          0x13
#define PIXI_GPI_IRQ_MODE_16_19         0x14
#define PIXI_DAC_PRESET_DATA1           0x16
#define PIXI_DAC_PRESET_DATA2           0x17
#define PIXI_TEMP_MON_CONFIG            0x18
#define PIXI_TEMP_INT_HIGH_THRESHOLD    0x19
#define PIXI_TEMP_INT_LOW_THRESHOLD     0x1A
#define PIXI_TEMP_EXT1_HIGH_THRESHOLD   0x1B
#define PIXI_TEMP_EXT1_LOW_THRESHOLD    0x1C
#define PIXI_TEMP_EXT2_HIGH_THRESHOLD   0x1D
#define PIXI_TEMP_EXT2_LOW_THRESHOLD    0x1E

#define PIXI_PORT_CONFIG                0x20
#define PIXI_PORT0_CONFIG               0x20
#define PIXI_PORT1_CONFIG               0x21
#define PIXI_PORT2_CONFIG               0x22
#define PIXI_PORT3_CONFIG               0x23
#define PIXI_PORT4_CONFIG               0x24
#define PIXI_PORT5_CONFIG               0x25
#define PIXI_PORT6_CONFIG               0x26
#define PIXI_PORT7_CONFIG               0x27
#define PIXI_PORT8_CONFIG               0x28
#define PIXI_PORT9_CONFIG               0x29
#define PIXI_PORT10_CONFIG              0x2A
#define PIXI_PORT11_CONFIG              0x2B
#define PIXI_PORT12_CONFIG              0x2C
#define PIXI_PORT13_CONFIG              0x2D
#define PIXI_PORT14_CONFIG              0x2E
#define PIXI_PORT15_CONFIG              0x2F
#define PIXI_PORT16_CONFIG              0x30
#define PIXI_PORT17_CONFIG              0x31
#define PIXI_PORT18_CONFIG              0x32
#define PIXI_PORT19_CONFIG              0x33

#define PIXI_ADC_DATA                   0x40
#define PIXI_PORT0_ADC_DATA             0x40
#define PIXI_PORT1_ADC_DATA             0x41
#define PIXI_PORT2_ADC_DATA             0x42
#define PIXI_PORT3_ADC_DATA             0x43
#define PIXI_PORT4_ADC_DATA             0x44
#define PIXI_PORT5_ADC_DATA             0x45
#define PIXI_PORT6_ADC_DATA             0x46
#define PIXI_PORT7_ADC_DATA             0x47
#define PIXI_PORT8_ADC_DATA             0x48
#define PIXI_PORT9_ADC_DATA             0x49
#define PIXI_PORT10_ADC_DATA            0x4A
#define PIXI_PORT11_ADC_DATA            0x4B
#define PIXI_PORT12_ADC_DATA            0x4C
#define PIXI_PORT13_ADC_DATA            0x4D
#define PIXI_PORT14_ADC_DATA            0x4E
#define PIXI_PORT15_ADC_DATA            0x4F
#define PIXI_PORT16_ADC_DATA            0x50
#define PIXI_PORT17_ADC_DATA            0x51
#define PIXI_PORT18_ADC_DATA            0x52
#define PIXI_PORT19_ADC_DATA            0x53

#define PIXI_DAC_DATA                   0x60
#define PIXI_PORT0_DAC_DATA             0x60
#define PIXI_PORT1_DAC_DATA             0x61
#define PIXI_PORT2_DAC_DATA             0x62
#define PIXI_PORT3_DAC_DATA             0x63
#define PIXI_PORT4_DAC_DATA             0x64
#define PIXI_PORT5_DAC_DATA             0x65
#define PIXI_PORT6_DAC_DATA             0x66
#define PIXI_PORT7_DAC_DATA             0x67
#define PIXI_PORT8_DAC_DATA             0x68
#define PIXI_PORT9_DAC_DATA             0x69
#define PIXI_PORT10_DAC_DATA            0x6A
#define PIXI_PORT11_DAC_DATA            0x6B
#define PIXI_PORT12_DAC_DATA            0x6C
#define PIXI_PORT13_DAC_DATA            0x6D
#define PIXI_PORT14_DAC_DATA            0x6E
#define PIXI_PORT15_DAC_DATA            0x6F
#define PIXI_PORT16_DAC_DATA            0x70
#define PIXI_PORT17_DAC_DATA            0x71
#define PIXI_PORT18_DAC_DATA            0x72
#define PIXI_PORT19_DAC_DATA            0x73

// Detailed register content map
// reg 0x00 Device ID
/* #define DEVID           0xFFFF */
// reg00x10 Device control
#define ADCCTL          0x0003
#define DACCTL		0x000C
#define ADCCONV	0x0030
#define DACREF		0x0040
#define THSHDN		0x0080
#define TMPCTL		0x0700
#define TMPCTLINT	0x0100
#define TMPCTLEXT1	0x0200
#define TMPCTLEXT2	0x0400
#define TMPPER		0x0800
#define RS_CANCEL	0x1000
#define LPEN		0x2000
#define BRST		0x4000
#define PIXI_RESET	0x8000
//ADCCTL values
#define ADC_MODE_IDLE   0x0
#define ADC_MODE_SWEEP  0x1
#define ADC_MODE_CONV   0x2
#define ADC_MODE_CONT   0x3

// reg 0x18 Temperature monitor config
#define TMPINTMONCFG    0x0003
#define TMPEXT1MONCFG   0x000C
#define TMPEXT2MONCFG   0x0030
// reg 0x19-1E Temperature monitor threshold high and low
#define TMPINTHI        0x0FFF
#define TMPINTLO        0x0FFF
#define TMPEXT1HI       0x0FFF
#define TMPEXT1LO       0x0FFF
#define TMPEXT2HI       0x0FFF
#define TMPEXT2LO       0x0FFF

// reg 0x20-33 Port Configuration
#define FUNCPRM         0x0FFF
#define FUNCID          0xF000
// Port Configuration register bits
#define FUNCPRM_ASSOCIATED_PORT    0x001F
#define FUNCPRM_NR_OF_SAMPLES      0x00E0
#define FUNCPRM_RANGE              0x0700
#define FUNCPRM_AVR_INV            0x0800
#define FUNCID_MODE0_HIGHZ         0x0000



// reg 0x40-53  ADC data
#define ADCDAT          0x0FFF
// reg 0x60-73  DAC data
#define DACDAT          0x0FFF

// Channel placeholder
#define CHANNEL_0       0x00
#define CHANNEL_1       0x01
#define CHANNEL_2       0x02
#define CHANNEL_3       0x03
#define CHANNEL_4       0x04
#define CHANNEL_5       0x05
#define CHANNEL_6       0x06
#define CHANNEL_7       0x07
#define CHANNEL_8       0x08
#define CHANNEL_9       0x09
#define CHANNEL_10      0x0a
#define CHANNEL_11      0x0b
#define CHANNEL_12      0x0c
#define CHANNEL_13      0x0d
#define CHANNEL_14      0x0e
#define CHANNEL_15      0x0f
#define CHANNEL_16      0x10
#define CHANNEL_17      0x11
#define CHANNEL_18      0x12
#define CHANNEL_19      0x13

// Channel mode placeholder
#define CH_MODE_0               0x00
#define CH_MODE_HIZ             0x00
#define CH_MODE_1               0x01
#define CH_MODE_GPI             0x01
#define CH_MODE_2               0x02
#define CH_MODE_DIDIR_LT_TERM   0x02
#define CH_MODE_3               0x03
#define CH_MODE_GPO_REG         0x03
#define CH_MODE_4               0x04
#define CH_MODE_GPO_UNI         0x04
#define CH_MODE_5               0x05
#define CH_MODE_DAC             0x05
#define CH_MODE_6               0x06
#define CH_MODE_DAC_ADC_MON     0x06
#define CH_MODE_7               0x07
#define CH_MODE_ADC_P           0x07
#define CH_MODE_8               0x08
#define CH_MODE_ADC_DIFF_P      0x08
#define CH_MODE_9               0x09
#define CH_MODE_ADC_DIFF_N      0x09
#define CH_MODE_10              0x0a
#define CH_MODE_DAC_ADC_DIFF_N  0x0a
#define CH_MODE_11              0x0b
#define CH_MODE_TERM_GPI_SW     0x0b
#define CH_MODE_12              0x0c
#define CH_MODE_TERM_REG_SW     0x0c

// Channel range
#define CH_NO_RANGE              0x0000
#define CH_0_TO_10P              0x0001
#define CH_5N_TO_5P              0x0002
#define CH_10N_TO_0              0x0003
#define CH_0_TO_2P5_5N_TO_5P     0x0004
/* #define CH_RES                   0x0005 */
#define CH_0_TO_2P5_0_TO_10P     0x0006
/* #define CH_RES                   0x0007 */


#define TEMP_CHANNEL_INT	0x0
#define TEMP_CHANNEL_EXT0	0x1
#define TEMP_CHANNEL_EXT1	0x2

class Pixi
{
  public:
    Pixi();
    void begin();
    uint16_t ReadRegister (uint8_t address, bool debug);
    void WriteRegister (uint8_t address, uint16_t value);
    uint16_t config();
    uint16_t configChannel(int channel, int channel_mode, uint16_t dac_dat, uint16_t range, uint8_t adc_ctl );
//    void configTempChannel(int channel);
//    void configInterrupt();
//    void readInterrupt();
    uint16_t readRawTemperature(int temp_channel);
    float readTemperature(int temp_channel);
    uint16_t readAnalog(int channel);
    uint16_t writeAnalog(int channel, uint16_t value);
  private:
    int _channel;
    int _temp_channel;
};

#endif  // PIXI_H
