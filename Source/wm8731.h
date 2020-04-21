#ifndef WM8731_H
#define WM8731_H

#include <stdint.h>
#include "device.h"
#include "Codec.h"

#define AUDIO_PROTOCOL               I2S_PROTOCOL_PHILIPS

/* ------------------------------------------------------------ */
/* WM8731 Registers */

#define LEFT_LINE_IN_REGISTER                   0x00
#define RIGHT_LINE_IN_REGISTER                  0x01
#define LEFT_HEADPHONE_OUT_REGISTER             0x02
#define RIGHT_HEADPHONE_OUT_REGISTER            0x03
#define ANALOGUE_AUDIO_PATH_CONTROL_REGISTER    0x04
#define DIGITAL_AUDIO_PATH_CONTROL_REGISTER     0x05
#define POWER_DOWN_CONTROL_REGISTER             0x06
#define DIGITAL_AUDIO_INTERFACE_FORMAT_REGISTER 0x07
#define SAMPLING_CONTROL_REGISTER               0x08
#define ACTIVE_CONTROL_REGISTER                 0x09
#define RESET_CONTROL_REGISTER                  0x0f

/* Register 0x00 and 0x01: Left/Right Line In */
#define WM8731_INVOL_P6DB    0x01b
#define WM8731_INVOL_0DB     0x017
#define WM8731_INVOL_M6DB    0x013
#define WM8731_INMUTE       (1<<7)
#define WM8731_INBOTH       (1<<8)

/* Register 0x02 and 0x03: Left/Right Headphone Out */
#define WM8731_HPVOL_P6DB    0x07f
#define WM8731_HPVOL_0DB     0x079
#define WM8731_HPVOL_M6DB    0x073
#define WM8731_HPVOL_M73DB   0x030
#define WM8731_HPVOL_MUTE    0x000
#define WM8731_ZCEN         (1<<7)
#define WM8731_HPBOTH       (1<<8)

/* Register 0x04: Analogue Audio Path Control */
#define WM8731_MICBOOST      (1<<0)     /* Microphone Input Level Boost Enable */
#define WM8731_MUTEMIC       (1<<1)     /* Mic Input Mute to ADC */
#define WM8731_INSEL         (1<<2)     /* Microphone Input Enable, Line Input Disable */
#define WM8731_BYPASS        (1<<3)     /* Bypass Enable */
#define WM8731_DACSEL        (1<<4)     /* DAC Enable */
#define WM8731_SIDETONE      (1<<5)     /* Side Tone Enable */
#define WM8731_SIDEATT_M15DB (0x03<<6)  /* Side Tone Attenuation -15dB */
#define WM8731_SIDEATT_M12DB (0x02<<6)  /* Side Tone Attenuation -12dB */
#define WM8731_SIDEATT_M9DB  (0x01<<6)  /* Side Tone Attenuation -9dB */
#define WM8731_SIDEATT_M6DB  (0x00<<6)  /* Side Tone Attenuation -6dB */

/* Register 0x05: Digital Audio Path Control */
#define WM8731_ADCHPD        (1<<0)     /* ADC High Pass Filter Disable */
#define WM8731_DEEMP_48K     (0x03<<1)  /* De-emphasis Control 48kHz */
#define WM8731_DEEMP_44K1    (0x02<<1)  /* De-emphasis Control 44.1kHz */
#define WM8731_DEEMP_32K     (0x01<<1)  /* De-emphasis Control 32kHz */
#define WM8731_DEEMP_NONE     0x00      /* De-emphasis Control Disabled */
#define WM8731_DACMU         (1<<3)     /* DAC Soft Mute */
#define WM8731_HPOR          (1<<4)     /* Store dc offset when High Pass Filter disabled */

/* Register 0x06: Power Down Control */
#define WM8731_LINEINPD      (1<<0)
#define WM8731_MICPD         (1<<1)
#define WM8731_ADCPD         (1<<2)
#define WM8731_DACPD         (1<<3)
#define WM8731_OUTPD         (1<<4)
#define WM8731_OSCPD         (1<<5)
#define WM8731_CLKOUTPD      (1<<6)
#define WM8731_POWEROFF      (1<<7)

/* Register 0x07: Digital Audio Interface Format */
#define WM8731_FORMAT_DSP     0x03      /* Audio Data Format: DSP Mode, frame sync + 2 data packed words */
#define WM8731_FORMAT_I2S     0x02      /* Audio Data Format: I2S Mode, MSB-First left-1 justified */
#define WM8731_FORMAT_MSB_LJ  0x01      /* Audio Data Format: MSB-First, left justified */
#define WM8731_FORMAT_MSB_RJ  0x00      /* Audio Data Format: MSB-First, right justified */
#define WM8731_IWL_16BIT      0x00      /* Input Word Length 16 bits */
#define WM8731_IWL_20BIT     (0x01<<2)  /* Input Word Length 20 bits */
#define WM8731_IWL_24BIT     (0x02<<2)  /* Input Word Length 24 bits */
#define WM8731_IWL_32BIT     (0x03<<2)  /* Input Word Length 32 bits */
#define WM8731_LRP           (1<<4)     /* DACLRC phase control */
#define WM8731_LRSWAP        (1<<5)     /* DAC Left Right Clock Swap */
#define WM8731_MS            (1<<6)     /* Master Mode Enable / Slave Mode Disable */
#define WM8731_BCLKINV       (1<<7)     /* Bit Clock Invert */

/* Register 0x08: Sampling Control */
#define WM8731_MODE_NORMAL    0x00
#define WM8731_MODE_USB       0x01
#define WM8731_BOSR          (1<<1)     /* Base Oversampling Rate: 0=256fs, 1=384fs */
#define WM8731_SR_48_48      (0x00<<2)  /* Normal mode rates, MCLK = 12.288MHz, 256fs  */
#define WM8731_SR_48_08      (0x01<<2)
#define WM8731_SR_08_48      (0x02<<2)
#define WM8731_SR_08_08      (0x03<<2)
#define WM8731_SR_32_32      (0x06<<2)
#define WM8731_SR_96_96      (0x07<<2)
#define WM8731_SR_USB_48_48  (0x00<<2)  /* USB mode rates */
#define WM8731_SR_USB_44_44  (0x08<<2)
#define WM8731_SR_USB_48_08  (0x01<<2)
#define WM8731_SR_USB_44_08  (0x09<<2)
#define WM8731_SR_USB_08_48  (0x02<<2)
#define WM8731_SR_USB_08_44  (0x0a<<2)
#define WM8731_SR_USB_08_08  (0x03<<2)
#define WM8731_SR_USB_32_32  (0x06<<2)
#define WM8731_SR_USB_96_96  (0x07<<2)
#define WM8731_CLKIDIV2      (1<<6)     /* Core Clock is MCLK divided by 2 */
#define WM8731_CLKODIV2      (1<<7)     /* CLOCKOUT is Core Clock divided by 2 */

/* Register 0x09: Active Control */
#define WM8731_ACTIVE        (1<<0)     /* Activate Interface */
#define WM8731_NOT_ACTIVE    (0<<0)     /* Activate Interface */

/* Register 0x0f: Reset Register */
#define WM8731_RESET          0x00      /* Reset Register */

#define WM8731_ADDR_0 0x1A
#define WM8731_ADDR_1 0x1B
/* The 7 bits Codec address (sent through I2C interface) */
#define CODEC_ADDRESS           (WM8731_ADDR_0<<1)
/* #define CODEC_ADDRESS           (WM8731_ADDR_0) */

#define WM8731_NUM_REGS 10
/* static uint16_t wm8731_registers[WM8731_NUM_REGS]; */

static const uint16_t wm8731_init_data[] = {
#ifdef OWL_MODULAR
  WM8731_INVOL_0DB,                   			  // Reg 0x00: Left Line In
  WM8731_INVOL_0DB,			                  // Reg 0x01: Right Line In
  WM8731_HPVOL_0DB,			                  // Reg 0x02: Left Headphone out
  WM8731_HPVOL_0DB,	                                  // Reg 0x03: Right Headphone out
#else
  WM8731_INVOL_P6DB,                   			  // Reg 0x00: Left Line In
  WM8731_INVOL_P6DB,			                  // Reg 0x01: Right Line In
  WM8731_HPVOL_M6DB,			                  // Reg 0x02: Left Headphone out
  WM8731_HPVOL_M6DB,	                                  // Reg 0x03: Right Headphone out
#endif
  WM8731_MUTEMIC|WM8731_DACSEL,                           // Reg 0x04: Analog Audio Path Control
#ifdef OWL_MODULAR
  WM8731_ADCHPD|WM8731_DEEMP_NONE,                        // Reg 0x05: Digital Audio Path Control
#else
  WM8731_DEEMP_NONE,                                      // Reg 0x05: Digital Audio Path Control
#endif
  WM8731_MICPD|WM8731_CLKOUTPD|WM8731_OSCPD|WM8731_OUTPD, // Reg 0x06: Power Down Control
  WM8731_MS|WM8731_FORMAT_I2S|WM8731_IWL_24BIT,           // Reg 0x07: Digital Audio Interface Format
  WM8731_MODE_NORMAL|WM8731_SR_48_48,                     // Reg 0x08: Sampling Control
  WM8731_NOT_ACTIVE                    			  // Reg 0x09: Active Control
/*      0x017,                  // Reg 00: Left Line In (0dB, mute off) */
/*      0x017,                  // Reg 01: Right Line In (0dB, mute off) */
/*      0x079,                  // Reg 02: Left Headphone out (0dB) */
/*      0x079,                  // Reg 03: Right Headphone out (0dB) */
/* 	0x012,			// Reg 04: Analog Audio Path Control (DAC sel, Mute Mic) */
/* 	0x000,			// Reg 05: Digital Audio Path Control */
/* 	0x062,			// Reg 06: Power Down Control (Clkout, Osc, Mic Off) */
/* 	0x002,			// Reg 07: Digital Audio Interface Format (i2s, 16-bit, slave) */
/* 	0x042,                  // Reg 07: i2s, 16bit, master */
/* 	0x000,			// Reg 08: Sampling Control (Normal, 256x, 48k ADC/DAC) */
/* 	0x001			// Reg 09: Active Control */
};

#endif /* WM8731_H */
