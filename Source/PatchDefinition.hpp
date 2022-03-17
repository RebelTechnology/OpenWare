#ifndef __PatchDefinition_hpp__
#define __PatchDefinition_hpp__

#include "device.h"
#include "message.h"
#include "ProgramVector.h"
#include "ProgramHeader.h"
#include "Storage.h"
#include "crc32.h"

class PatchDefinition {
private:
  uint32_t* stackBase;
  uint32_t stackSize;
  ProgramVector* programVector;
  typedef void (*ProgramFunction)(void);
  ProgramFunction programFunction;
  uint32_t* linkAddress;
  uint32_t binarySize;
  uint32_t programSize;
  char programName[16]; // ResourceHeader::name is char[16], ProgramHeader::name is char[24]
  ResourceHeader* sourceResource;
  
  bool load(ProgramHeader* header, uint32_t sz){
    binarySize = sz;
    // check we've got an entry function
    if(header->magic != 0xDADAC0DE)
      return false;
    linkAddress = header->linkAddress;
    programSize = (uint32_t)header->endAddress - (uint32_t)header->linkAddress;
    stackBase = header->stackBegin;
    stackSize = (uint32_t)header->stackEnd - (uint32_t)header->stackBegin;
    programVector = header->programVector;
    strlcpy(programName, header->programName, sizeof(programName));
    programFunction = (ProgramFunction)header->jumpAddress;
    return true;
  }
public:
  PatchDefinition() {
    reset();
  }
  void reset(){
    programVector = NULL;
    programFunction = NULL;
    sourceResource = NULL;
  }
  bool copy(){
    bool ret = false;
    if(sourceResource){
      uint32_t checksum = sourceResource->checksum;
      storage.readResource(sourceResource, linkAddress, 0, binarySize);
      device_cache_invalidate();
      uint32_t crc = crc32(linkAddress, binarySize, 0);
      if(crc == checksum){
	ret = true;
      }else{
	if(checksum != storage.getChecksum(sourceResource))
	  error(PROGRAM_ERROR, "Invalid checksum");
      }
      sourceResource = NULL;
    }
    return ret;
  }
  uint32_t* getStackBase(){
    return stackBase;
  }
  uint32_t getStackSize(){
    return stackSize;
  }
  const char* getName(){
    return programName;
  }
  ProgramVector* getProgramVector(){
    return programVector;
  }
  bool load(ResourceHeader* resource){
    reset();
    ProgramHeader header;
    storage.readResource(resource, &header, 0, sizeof(header));
    if(load(&header, resource->size)){
      sourceResource = resource;
      return true;
    }
    return false;
  }
  bool isValid(){
    // if(programSize != binarySize)
    //   error(PROGRAM_ERROR, "Invalid program size");
    // if(programSize != binarySize)
    //   return false;
    if(programFunction == NULL || programVector == NULL)
      return false;
    extern char _PATCHRAM, _PATCHRAM_SIZE;
    if(linkAddress == (uint32_t*)&_PATCHRAM)
      return programSize <= (uint32_t)(&_PATCHRAM_SIZE);
#ifdef USE_PLUS_RAM
    extern char _PLUSRAM, _PLUSRAM_SIZE;
    if(linkAddress == (uint32_t*)&_PLUSRAM)
      return programSize <= (uint32_t)(&_PLUSRAM_SIZE);
#endif
    return false;
  }
  void run(){
    // check magic
    if((*linkAddress) == 0xDADAC0DE){
      if(binarySize < programSize) // blank out bss area
	memset((uint8_t*)linkAddress + binarySize, 0, programSize - binarySize);
      device_cache_invalidate();
      // memory barriers for dynamically loaded code
      // __DSB(); __ISB();
      programFunction();
    }
  }
  uint32_t getProgramSize(){
    return programSize;
  }
  uint32_t getBinarySize(){
    return binarySize;
  }
  uint32_t* getLinkAddress(){
    return linkAddress;
  }
};


#endif // __PatchDefinition_hpp__
