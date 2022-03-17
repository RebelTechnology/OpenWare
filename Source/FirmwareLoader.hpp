#ifndef __FirmwareLoader_H__
#define __FirmwareLoader_H__

#include <math.h>
#include "crc32.h"
#include "sysex.h"
#include "device.h"
#include <stdint.h>
#include "errorhandlers.h"
#include "ProgramManager.h"
#include "ResourceHeader.h"

class FirmwareLoader {
public:
  size_t packageIndex = 0;
  uint8_t* buffer = NULL;
  size_t size;
  size_t index;
  uint32_t crc;
  bool ready;
public:
  void clear(){
    buffer = NULL;
    index = 0;
    packageIndex = 0;
    ready = false;
    crc = 0;
  }

  uint32_t getChecksum(){
    return crc;
  }

  bool isReady(){
    return ready;
  }

  int setError(const char* msg){
    error(PROGRAM_ERROR, msg);
    clear();
    return -1;
  }

  size_t getDataSize(){
    return size;
  }

  size_t getLoadedSize(){
    return index - sizeof(ResourceHeader);
  }

  size_t getTotalSize(){
    return size + sizeof(ResourceHeader);
  }

  uint8_t* getData(){
    return buffer + sizeof(ResourceHeader);
  }

  ResourceHeader* getResourceHeader(){
    return (ResourceHeader*)buffer;
  }

  void allocateBuffer(size_t size){
#ifdef USE_EXTERNAL_RAM
    extern char _EXTRAM; // defined in link script
    buffer = (uint8_t*)&_EXTRAM;
#else
    // required by devices with no ext mem
    extern char _PATCHRAM;
    buffer = (uint8_t*)&_PATCHRAM; 
#endif
    memset(buffer, 0, sizeof(ResourceHeader));
    index = sizeof(ResourceHeader); // start writing data after resource header
  }
  
  /* decode a 32-bit unsigned integer from 5 bytes of sysex encoded data */
  uint32_t decodeInt(uint8_t *data){
    uint8_t buf[4];
    sysex_to_data(data, buf, 5);
    uint32_t result = buf[3] | (buf[2] << 8L) | (buf[1] << 16L) | (buf[0] << 24L);
    return result;
  }

  int32_t beginFirmwareUpload(uint8_t* data, size_t length, size_t offset){
    clear();
    setErrorStatus(NO_ERROR);
    // first package
    if(length < 3+5+5)
      return setError("Invalid SysEx package");
    // stop running program and free its memory
    program.exitProgram(true);
#ifndef USE_BOOTLOADER_MODE
    owl.setOperationMode(LOAD_MODE);
#endif
    // get firmware data size (decoded)
    size = decodeInt(data+offset);
    offset += 5; // it takes five 7-bit values to encode four bytes
    // allocate memory
    if(size > MAX_SYSEX_PAYLOAD_SIZE)
      return setError("SysEx too big");
    allocateBuffer(size);
    packageIndex = 1;
    return 0;
  }

  int32_t receiveFirmwarePackage(uint8_t* data, size_t length, size_t offset){
    size_t len = sysex_to_data(data+offset, buffer+index, length-offset);
    crc = crc32(buffer+index, len, crc);
    index += len;
    packageIndex++;
    return 0;
  }

  int32_t finishFirmwareUpload(uint8_t* data, size_t length, size_t offset){
    // last package: index and checksum
    // crc = crc32(getData(), getDataSize(), 0);
    if(length < 5)
      return setError("Missing checksum");
    uint32_t checksum = decodeInt(data+offset);
    if(crc != checksum)
      return setError("Invalid SysEx checksum");
    ready = true;
    return index;
  }

  bool setResourceName(const char* name){
    ResourceHeader* header = getResourceHeader();
    size_t len = strnlen(name, sizeof(ResourceHeader::name));
    if(len > 0 && len < sizeof(ResourceHeader::name)){
      size_t datasize = getDataSize();
      if(datasize <= MAX_SYSEX_PAYLOAD_SIZE){
	storage.writeResourceHeader(header, name, datasize, crc, RESOURCE_IN_MEMORY); // FLASH_DEFAULT_FLAGS);
	return true;
      }else{
	setError("Resource too big");
      }
    }else{
      setError("Invalid SAVE name");
    }
    return false;
  }

  bool setPatchSlot(uint8_t slot){
    ResourceHeader* header = getResourceHeader();
    ProgramHeader* program = (ProgramHeader*)getData();
    const char* name = program->programName;
    size_t datasize = getDataSize();
    if(slot > MAX_NUMBER_OF_PATCHES){
      setError("Invalid STORE slot");
    }else if(program->magic != 0XDADAC0DE){
      setError("Invalid patch magic");
    }else if(datasize > MAX_SYSEX_PROGRAM_SIZE){
      setError("Patch too big");
    }else{
      storage.writeResourceHeader(header, name, datasize, crc, RESOURCE_IN_MEMORY|RESOURCE_USER_PATCH|slot);
				  // FLASH_DEFAULT_FLAGS|RESOURCE_USER_PATCH|slot);
      return true;
    }
    return false;
  }

  int32_t handleFirmwareUpload(uint8_t* data, size_t length){
    size_t offset = 3;
    size_t idx = decodeInt(data+offset);
    offset += 5;
    if(idx == 0)
      return beginFirmwareUpload(data, length, offset); // first package
    else if(packageIndex != idx)
      return setError("SysEx package out of sequence"); // out of sequence package
    else if(getLoadedSize() < getDataSize())
      return receiveFirmwarePackage(data, length, offset); // mid transfer package
    else if(getLoadedSize() == getDataSize())
      return finishFirmwareUpload(data, length, offset); // last package
    else
      return setError("Invalid SysEx size"); // wrong size
  }
};

#endif // __FirmwareLoader_H__
