#ifndef __BUS_H
#define __BUS_H

#include <stdint.h>
#define DIGITAL_BUS_BUFFER_SIZE 512

#ifdef __cplusplus
#include "ApplicationSettings.h"

 extern "C" {
#endif

#define BUS_STATUS_IDLE        0x00
#define BUS_STATUS_DISCOVER    0x01
#define BUS_STATUS_CONNECTED   0x02
#define BUS_STATUS_ERROR       0xff

#define BUS_CMD_CONFIGURE_IO   0x10
  
   void bus_setup();
   int bus_status();
   uint8_t* bus_deviceid();
   /* outgoing: send message over digital bus */
   void bus_tx_parameter(uint8_t pid, int16_t value);
   /* incoming: callback when message received on digital bus */
   void bus_rx_parameter(uint8_t pid, int16_t value);
   void bus_tx_button(uint8_t bid, int16_t value);
   void bus_rx_button(uint8_t bid, int16_t value);
   void bus_tx_command(uint8_t cmd, int16_t data);
   void bus_rx_command(uint8_t cmd, int16_t data);
   void bus_tx_message(const char* msg);
   void bus_rx_message(const char* msg); 
   void bus_tx_data(const uint8_t* data, uint16_t size);
   void bus_rx_data(const uint8_t* data, uint16_t size); 
   void bus_tx_error(const char* reason);
   void bus_rx_error(const char* reason);
   void bus_set_input_channel(uint8_t ch);

   void bus_tx_frame(uint8_t* data);
   void bus_write(uint8_t* data, uint16_t len);
   
   void initiateBusRead();
   void initiateBusWrite();
#ifdef __cplusplus
}
#endif

#endif /* __BUS_H */
