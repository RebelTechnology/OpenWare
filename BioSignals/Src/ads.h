#ifndef __ADS_H
#define __ADS_H

#include <stdint.h>
#include <stddef.h>

void ads_send_command(int cmd);
void ads_write_reg(int reg, int val);
int ads_read_reg(int reg);
int ads_read_single_sample();
void ads_sample(int32_t* samples, size_t len);

#endif /* __ADS_H */
