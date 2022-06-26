#ifndef _CircularBuffer_hpp_
#define _CircularBuffer_hpp_

#include <stdint.h>
#include <string.h> // for memcpy

#ifndef FLOW_ASSERT
#define FLOW_ASSERT(x, y)
#endif

template<typename DataType, typename IndexType = size_t>
class CircularBuffer {
protected:
  DataType* data;
  IndexType size;
  IndexType writepos = 0;
  IndexType readpos = 0;
  bool empty = true;
public:
  CircularBuffer(): data(NULL), size(0){}
  CircularBuffer(DataType* data, IndexType size): data(data), size(size){}

  void setData(DataType* data, IndexType len) {
    this->data = data;
    size = len;
  }

  IndexType getSize() const {
    return size;
  }

  DataType* getData() {
    return data;
  }
  
  bool isEmpty() const {
    return empty;
  }
  
  bool isFull() const {
    return (writepos == readpos) && !empty;
  }

  void write(DataType c){
    FLOW_ASSERT(getWriteCapacity() > 0, "overflow");
    data[writepos++] = c;
    if(writepos >= size)
      writepos = 0;
    empty = false;
  }

  void write(DataType* source, IndexType len){
    FLOW_ASSERT(getWriteCapacity() >= len, "overflow");
    DataType* dest = getWriteHead();
    IndexType rem = size-writepos;
    if(len >= rem){
      memcpy(dest, source, rem*sizeof(DataType));
      writepos = len-rem;
      memcpy(data, source+rem, writepos*sizeof(DataType));
    }else{
      memcpy(dest, source, len*sizeof(DataType));
      writepos += len;
    }
    empty = false;
  }
    
  void writeAt(IndexType index, DataType value){
    data[index % size] = value;
  }

  void overdub(DataType c){
    data[writepos++] += c;
    if(writepos >= size)
      writepos = 0;
    empty = false;
  }

  void overdubAt(IndexType index, DataType value){
    data[index % size] += value;
  }

  DataType read(){
    FLOW_ASSERT(getReadCapacity() > 0, "underflow");
    DataType c = data[readpos++];
    if(readpos >= size)
      readpos = 0;
    empty = readpos == writepos;
    return c;
  }

  void read(DataType* dst, IndexType len){
    FLOW_ASSERT(getReadCapacity() >= len, "underflow");
    DataType* src = getReadHead();
    IndexType rem = size-readpos;
    if(len > rem){
      memcpy(dst, src, rem*sizeof(DataType));
      readpos = len-rem;
      memcpy(dst+rem, data, readpos*sizeof(DataType));
    }else{
      memcpy(dst, src, len*sizeof(DataType));
      readpos += len;
    }
    empty = readpos == writepos;
  }
  
  DataType readAt(IndexType index){
    return data[index % size];
  }

  void skipUntilLast(char c){
    DataType* src = getReadHead();
    IndexType rem = size-readpos;
    for(int i=0; i<rem; ++i){
      if(src[i] != c){
	readpos += i;
	return;
      }
    }
    rem = writepos;
    for(int i=0; i<rem; ++i){
      if(data[i] != c){
	readpos = i;
	return;
      }
    }
    empty = readpos == writepos;
  }

  IndexType getWriteIndex(){
    return writepos;
  }

  void setWriteIndex(IndexType pos){
    writepos = pos % size;
  }

  DataType* getWriteHead(){
    return data+writepos;
  }

  void moveWriteHead(int32_t samples){
    FLOW_ASSERT(getWriteCapacity() >= samples, "overflow");
    writepos = (writepos + samples) % size;
    empty = false;
  }

  IndexType getReadIndex(){
    return readpos;
  }

  void setReadIndex(IndexType pos){
    readpos = pos % size;
  }

  DataType* getReadHead(){
    return data+readpos;
  }

  void moveReadHead(int32_t samples){
    FLOW_ASSERT(getReadCapacity() < samples, "underflow");
    readpos = (readpos + samples) % size;
    empty = readpos == writepos;
  }

  /**
   * Set the read index @param samples behind the write index.
   */
  void setDelay(int samples){
    readpos = (writepos-samples+size) % size;
  }

  /**
   * Get the read index expressed as delay behind the write index.
   */
  IndexType getDelay() const {
    return (writepos-readpos+size) % size;
  }

  /**
   * Write to buffer and read with a delay
   */
  void delay(DataType* in, DataType* out, IndexType len, int delay_samples){
    setDelay(delay_samples); // set delay relative to where we start writing
    write(in, len);
    read(out, len);
  }

  IndexType getReadCapacity() const {
    return size - getWriteCapacity();
  }

  IndexType getWriteCapacity() const {
    return size*empty + (readpos + size - writepos) % size;
  }

  IndexType getContiguousWriteCapacity() const {
    if(writepos < readpos)
      return readpos - writepos;
    else
      return size - writepos;
  }

  IndexType getContiguousReadCapacity() const {
    if(writepos > readpos)
      return writepos - readpos;
    else
      return size - readpos;
  }

  void setAll(const DataType value){
    for(IndexType i=0; i<size; ++i)
      data[i] = value;
  }

  void reset(){
    readpos = writepos = 0;
    empty = true;
  }

  void clear(){
    setAll(0);
  }

  static CircularBuffer<DataType>* create(IndexType len){
    CircularBuffer<DataType>* obj = new CircularBuffer<DataType>(new DataType[len], len);
    obj->clear();
    return obj;
  }

  static void destroy(CircularBuffer<DataType>* obj){
    delete[] obj->data;
    delete obj;
  }
};

typedef CircularBuffer<float> CircularFloatBuffer;
typedef CircularBuffer<int16_t> CircularShortBuffer;
typedef CircularBuffer<int32_t> CircularIntBuffer;

#endif /* _CircularBuffer_hpp_ */
