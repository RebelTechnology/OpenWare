#include <inttypes.h>

#include "mxconstants.h"

#define FIRMWARE_VERSION             "002"
#define HARDWARE_VERSION             "Expander Rev01"

#define MIDI_MAX_MESSAGE_SIZE        256

/**
 * MAX11300 on SPI1
 * TLC5946 on SPI2
 * Digital Bus on USART1
 */
/* #define USART_BAUDRATE               115200 */
/* #define USART_PERIPH                 USART1 */

#ifdef  USE_FULL_ASSERT
#ifdef __cplusplus
 extern "C" {
#endif
   void assert_failed(uint8_t* file, uint32_t line);
#ifdef __cplusplus
}
#endif
#define assert_param(expr) ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, __LINE__))
#endif
