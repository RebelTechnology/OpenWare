#ifndef __ADS_H
#define __ADS_H

#include <stdint.h>
#include <stddef.h>

#define ADS_MAX_CHANNELS 4
#define ADS_BLOCKSIZE 256

#ifdef __cplusplus
 extern "C" {
#endif

void ads_setup();
void ads_send_command(int cmd);
void ads_write_reg(int reg, int val);
int ads_read_reg(int reg);
int ads_read_single_sample();
void ads_sample(int32_t* samples, size_t len);
void ads_drdy();

void ads_set_gain(int gain);
void ads_start_continuous();
void ads_stop_continuous();

void ads_rx_callback(int32_t* samples, size_t channels, size_t blocksize);

#ifdef __cplusplus
}
#endif

#endif /* __ADS_H */
