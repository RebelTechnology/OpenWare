#ifndef __PatchDefinition_hpp__
#define __PatchDefinition_hpp__

#include "ProgramVector.h"
#include "ProgramHeader.h"
#include "Storage.h"

// todo: put implementation straight into PatchDefinition base class
// 'load' functions should be renamed
// 'verify' should be called 'load'
class PatchDefinition {
private:
  uint32_t* stackBase;
  uint32_t stackSize;
  ProgramVector* programVector = NULL;
  typedef void (*ProgramFunction)(void);
  ProgramFunction programFunction = NULL;
  uint32_t* linkAddress;
  uint32_t binarySize;
  uint32_t programSize;
  char programName[20]; // ResourceHeader::name is char[20]
  Resource* sourceResource = NULL;
  void* sourceAddress = NULL;
  
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
  PatchDefinition() {}
  void copy(){
    if(sourceResource){
      storage.readResource(sourceResource, linkAddress, 0, binarySize);
      sourceResource = NULL;
    }else if(sourceAddress){
      if(linkAddress != sourceAddress)
	memmove(linkAddress, sourceAddress, binarySize);
      sourceAddress = NULL;
    }
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
  bool load(Resource* resource){
    sourceAddress = NULL;
    sourceResource = NULL;
    ProgramHeader header;
    storage.readResource(resource, &header, 0, sizeof(header));
    if(load(&header, resource->getDataSize())){
      sourceResource = resource;
      return true;
    }
    return false;
  }
  // called on program RUN from RAM
  bool load(void* address, uint32_t sz){
    sourceAddress = NULL;
    sourceResource = NULL;
    ProgramHeader* header = (ProgramHeader*)address;
    if(load(header, sz)){
      sourceAddress = address;
      return true;
    }
    return false;
  }
  bool isValid(){
    // if(binarySize != programSize)
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
    if((*(uint32_t*)linkAddress) == 0xDADAC0DE){
      if(binarySize < programSize) // blank out bss area
	memset(linkAddress+binarySize, 0, programSize - binarySize);
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
