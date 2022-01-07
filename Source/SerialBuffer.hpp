#ifndef _SerialBuffer_hpp_
#define _SerialBuffer_hpp_

#include <stdint.h>
#include <string.h> // for memcpy

template<size_t size, typename T = uint8_t>
class SerialBuffer {
private:
  volatile size_t writepos = 0;
  volatile size_t readpos = 0;
  T buffer[size];
public:
  size_t getCapacity(){
    return size;
  }
  void push(T* data, size_t len){
    T* dest = getWriteHead();
    size_t rem = size-writepos;
    if(len >= rem){
      memcpy(dest, data, rem);
      // note that len-rem may be zero
      memcpy(buffer, data+rem, len-rem);
      writepos = len-rem;
    }else{
      memcpy(dest, data, len);
      writepos += len;
    }
  }
  void pull(T* data, size_t len){
    T* src = getReadHead();
    size_t rem = size-readpos;
    if(len >= rem){
      memcpy(data, src, rem);
      // note that len-rem may be zero
      memcpy(data+rem, buffer, len-rem);
      readpos = len-rem;
    }else{
      memcpy(data, src, len);
      readpos += len;
    }
  }

  void push(T c){
    buffer[writepos] = c;
    writepos = (writepos + 1) % size;
  }
  T pull(){
    T c = buffer[readpos];
    readpos = (readpos + 1) % size;
    return c;
  }

  void skipUntilLast(char c){
    T* src = getReadHead();
    size_t rem = size-readpos;
    for(size_t i=0; i<rem; ++i){
      if(src[i] != c){
	readpos += i;
	return;
      }
    }
    rem = writepos;
    for(size_t i=0; i<rem; ++i){
      if(buffer[i] != c){
	readpos = i;
	return;
      }
    }
  }
  size_t getWriteIndex(){
    return writepos;
  }
  void setWriteIndex(size_t pos){
    writepos = pos;
  }
  T* getWriteHead(){
    return buffer+writepos;
  }
  void incrementWriteHead(size_t len){
    // ASSERT((writepos >= readpos && writepos+len <= size) ||
    // 	   (writepos < readpos && writepos+len <= readpos), "uart rx overflow");
    writepos += len;
    if(writepos >= size)
      writepos -= size;
  }

  size_t getReadIndex(){
    return readpos;
  }
  void setReadIndex(size_t pos){
    readpos = pos;
  }
  T* getReadHead(){
    return buffer+readpos;
  }
  void incrementReadHead(size_t len){
    // ASSERT((readpos >= writepos && readpos+len <= size) ||
    // 	   (readpos < writepos && readpos+len <= writepos), "uart rx underflow");
    readpos = (readpos + len) % size;
  }
  bool notEmpty(){
    return writepos != readpos;
  }
  size_t getReadCapacity(){
    return (writepos + size - readpos) % size;
  }
  size_t getWriteCapacity(){
    return size - getReadCapacity();
  }
  size_t getContiguousWriteCapacity(){
    if(writepos < readpos)
      return readpos - writepos;
    else
      return size - writepos;
  }
  size_t getContiguousReadCapacity(){
    if(writepos < readpos)
      return size - readpos;
    else
      return writepos - readpos;
  }
  void reset(){
    readpos = writepos = 0;
    setAll(0);
  }
  void setAll(const T value){
    for(size_t i=0; i<size; ++i)
      buffer[i] = value;
  }
};

#endif /* _SerialBuffer_hpp_ */
