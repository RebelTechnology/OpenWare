#ifndef __ADS_H
#define __ADS_H

#include "device.h"

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
 extern "C" {
#endif

void ads_setup(int32_t* samples);
void ads_send_command(int cmd);
void ads_write_reg(int reg, int val);
int ads_read_reg(int reg);
int ads_read_single_sample(void);
void ads_sample(int32_t* samples, size_t len);
void ads_drdy(void);
void ads_cplt(void);

void ads_set_gain(int gain);
void ads_start_continuous(void);
void ads_stop_continuous(void);
uint32_t ads_get_status(void);
void ads_set_lod(uint8_t channels); // set lead off detection for channels, 0 for disabled
void ads_set_rld(uint8_t channels); // set right leg drive for channels, 0 for disabled

/* void ads_rx_callback(int32_t* samples, size_t channels, size_t blocksize); */

#ifdef __cplusplus
}
#endif

#endif /* __ADS_H */
