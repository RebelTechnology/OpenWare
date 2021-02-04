#ifndef __DynamicPatchDefinition_hpp__
#define __DynamicPatchDefinition_hpp__

#include "PatchDefinition.hpp"
#include "ProgramHeader.h"

class DynamicPatchDefinition : public PatchDefinition {
private:
  typedef void (*ProgramFunction)(void);
  ProgramFunction programFunction;
  uint32_t* linkAddress;
  uint32_t* jumpAddress;
  uint32_t* programAddress;
  uint32_t programSize;
  ProgramHeader* header;
  char programName[24];
public:
  DynamicPatchDefinition() :
    PatchDefinition(programName, 2, 2) {}
  DynamicPatchDefinition(void* addr, uint32_t sz) :
    PatchDefinition(programName, 2, 2) {
    load(addr, sz);
  }
  bool load(void* addr, uint32_t sz){
    programAddress = (uint32_t*)addr;
    header = (ProgramHeader*)addr;
    linkAddress = header->linkAddress;
    programSize = (uint32_t)header->endAddress - (uint32_t)header->linkAddress;
    if(sz != programSize)
      return false;
    stackBase = header->stackBegin;
    stackSize = (uint32_t)header->stackEnd - (uint32_t)header->stackBegin;
    jumpAddress = header->jumpAddress;
    programVector = header->programVector;
    strlcpy(programName, header->programName, sizeof(programName));
    programFunction = (ProgramFunction)jumpAddress;
    return true;
  }
  void copy(){
    extern char _PATCHRAM, _PATCHRAM_SIZE;
    /* copy program to ram */
    if((linkAddress == (uint32_t*)&_PATCHRAM && programSize <= (uint32_t)(&_PATCHRAM_SIZE))){
      memcpy((void*)linkAddress, (void*)programAddress, programSize);
      programAddress = linkAddress;
    }else{
      programFunction = NULL;
    }
  }
  bool verify(){
    // check we've got an entry function
    if(programFunction == NULL)
      return false;
    // check magic
    if((*(uint32_t*)programAddress & 0xffffff00) != 0xDADAC000) // was: != 0xDADAC0DE
      return false;
    return true;
  }
  void run(){
    if(linkAddress != programAddress)
      copy();
    if(programFunction != NULL)
      programFunction();
  }
  uint32_t getProgramSize(){
    return programSize;
  }
  uint32_t* getLinkAddress(){
    return linkAddress;
  }
};


#endif // __DynamicPatchDefinition_hpp__
