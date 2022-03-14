#ifndef __RESOURCE_HEADER_H
#define __RESOURCE_HEADER_H

#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

   struct ResourceHeader {
     uint32_t magic; // 0xDADADEED for resources
     uint32_t size;
     /* char shortname[12]; */
     /* char* name; */
     char name[16];
     uint32_t checksum;
     uint32_t flags;
   };

#ifdef __cplusplus
}
#endif

#endif /* __RESOURCE_HEADER_H */
