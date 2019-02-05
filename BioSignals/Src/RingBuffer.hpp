#ifndef _RINGBUFFER_H_
#define _RINGBUFFER_H_

// #include <cstring>
// #include <cassert>

template <typename T>
class RingBuffer
{
public:
  RingBuffer();
  RingBuffer(T* data, size_t n);
	
  void setBuffer(T* buffer, size_t len);
  
	inline size_t getSize() const;
  
  inline size_t getReadSpace() const;

  inline size_t getWriteSpace() const;

  inline size_t read(T* destination, size_t n, bool advance = false);

  inline size_t readInterleaved(T* destination, size_t n);

  inline size_t advance(size_t n);

  inline size_t write(const T *source, size_t n);
	
	inline size_t zero(size_t n);
    
  inline void reset() {
      m_writePosition = m_readPosition = 0;
  }

  inline RingBuffer<T>& push(T item){
    m_buffer[m_writePosition++] = item;
    if(m_writePosition >= m_size)
      m_writePosition = 0;
    return *this;
  }

  inline T pull(){
    T c = m_buffer[m_readPosition++];
    if(m_readPosition >= m_size)
      m_readPosition = 0;
    return c;
  }

  T* getReadHead(){
    return m_buffer+m_readPosition;
  }

  void setReadHead(T* ptr){
    size_t idx = ptr-m_buffer;
    if(idx >= 0 && idx < m_size)
      m_readPosition = idx;
  }

  T* getWriteHead(){
    return m_buffer+m_writePosition;
  }

  void setWriteHead(T* ptr){
    size_t idx = ptr-m_buffer;
    if(idx >= 0 && idx < m_size)
      m_writePosition = idx;
  }

  size_t getReadPos(){
    return m_readPosition;
  }

  size_t getWritePos(){
    return m_writePosition;
  }

  void setReadPos(size_t pos){
    m_readPosition = pos;
  }

  void setWritePos(size_t pos){
    m_writePosition = pos;
  }

  inline void incrementWriteHead(int len){
    m_writePosition = (m_writePosition + len) % m_size;
  }

  inline void incrementReadHead(int len){
    m_readPosition = (m_readPosition + len) % m_size;
  }

  inline size_t getWriteCapacity(){
    if(m_writePosition < m_readPosition)
      return m_readPosition - m_writePosition;
    else
      return m_size - m_writePosition + m_readPosition;
  }


  inline size_t getContiguousWriteCapacity(){
    if(m_writePosition < m_readPosition)
      return m_readPosition - m_writePosition;
    else
      return m_size - m_writePosition;
  }

  inline size_t getContiguousReadCapacity(){
    if(m_writePosition < m_readPosition)
      return m_size - m_readPosition;
    else
      return m_writePosition - m_readPosition;
  }

  inline bool notEmpty(){
    return m_writePosition != m_readPosition;
  }

  inline size_t available(){
    return (m_writePosition + m_size - m_readPosition) % m_size;
  }

  inline T* firstOccurence(T t){
    if(m_writePosition < m_readPosition){
      for(int i=m_readPosition;i<m_size; ++i){
	if(m_buffer[i] == t)
	  return &m_buffer[i];
      }
      for(int i=0; i<m_writePosition; ++i){
	if(m_buffer[i] == t)
	  return &m_buffer[i];
      }
    }else{
      for(int i=m_readPosition; i<m_writePosition; ++i){
	if(m_buffer[i] == t)
	  return &m_buffer[i];
      }
    }
    return NULL;
  }

  size_t readUntil(T *destination, size_t n, T end);

private:

    RingBuffer(const RingBuffer &);
    RingBuffer &operator=(const RingBuffer &);
    
	T      *m_buffer;
    volatile size_t  m_writePosition;
    volatile size_t  m_readPosition;
    size_t  m_size;	
};

template <typename T>
RingBuffer<T>::RingBuffer() :
  m_buffer(NULL),
  m_writePosition(0),
  m_readPosition(0),
  m_size(0)
{
}

template <typename T>
RingBuffer<T>::RingBuffer(T* buf, size_t n) :
    m_buffer(buf),
    m_writePosition(0),
    m_readPosition(0),
    m_size(n)
{
}

template <typename T>
void RingBuffer<T>::setBuffer(T* buffer, size_t len){
  m_buffer = buffer;
  m_size = len;
}


template <typename T>
size_t RingBuffer<T>::getSize() const
{
    return m_size - 1;
}

template <typename T>
size_t RingBuffer<T>::getReadSpace() const
{
    size_t space = (m_writePosition + m_size - m_readPosition) % m_size;

    return space;
}

template <typename T>
size_t
RingBuffer<T>::getWriteSpace() const
{
	//[0123R567W9]
	size_t space = (m_readPosition + m_size - m_writePosition - 1) % m_size;

    return space;
}

template <typename T>
size_t RingBuffer<T>::read(T *destination, size_t n, bool advance)
{
    // size_t available = getReadSpace();
    // if (n > available) 
    // {
    //   memset(destination, 0, (n - available) * sizeof(T));
    //   destination = destination + n - available;
    //   n = available;
    // }
    // if (n == 0) return n;

	//[0R234567W9*10*]
	//here = 11-1
    size_t here = m_size - m_readPosition;
    if (here >= n) 
    {
		memcpy(destination, m_buffer + m_readPosition, n * sizeof(T));
    } 
    else 
    {
		memcpy(destination, m_buffer + m_readPosition, here * sizeof(T));
		memcpy(destination + here, m_buffer, (n - here) * sizeof(T));
    }

    // commented out to manage overlap  
    if (advance) m_readPosition = (m_readPosition + n) % m_size;

    return n;
}


template <typename T>
size_t RingBuffer<T>::readUntil(T *destination, size_t n, T end)
{
  int i=0;
  if(m_writePosition < m_readPosition){
    for(; m_readPosition < m_size; ++i){
      destination[i] = m_buffer[m_readPosition++];
      if(i == n || destination[i] == end)
	return i;
    }
    m_readPosition = 0;
  }
  for(; m_readPosition < m_writePosition; ++i){
    destination[i] = m_buffer[m_readPosition++];
    if(i == n || destination[i] == end)
      return i;
  }
  return i;
}

template <typename T>
size_t RingBuffer<T>::readInterleaved(T *destination, size_t n)
{
    size_t available = getReadSpace();
    if (n > available) 
    {
      // underflow
      // assert(false);
    }
    if (n == 0) return n;

	//[0R234567W9*10*]
	//here = 11-1
    size_t here = m_size - m_readPosition;
    if (here >= n) 
    {
      for (int i = 0, ctr = 0; i < n; ++i) {
        destination[ctr++] = *(m_buffer + m_readPosition + i);
        destination[ctr++] = T(0);
      }
    } 
    else 
    {
      for (int i = 0, ctr = 0; i < here; ++i) {
        destination[ctr++] = *(m_buffer + m_readPosition + i);
        destination[ctr++] = T(0);
      }
      for (int i = 0, ctr = 0; i < n - here; ++i) {
        destination[ctr++] = *(m_buffer + i);
        destination[ctr++] = T(0);
      }
    }

    // commented out to manage overlap  
    //m_readPosition = (m_readPosition + n) % m_size;

    return n;
}


template <typename T>
size_t RingBuffer<T>::advance(size_t n)
{
    m_readPosition = (m_readPosition + n) % m_size;
    return n;
}


template <typename T>
size_t RingBuffer<T>::write(const T *source, size_t n)
{
    // size_t available = getWriteSpace();
    
    // if (n > available) 
    // {
      // overflow
	// assert(false);
	// 	n = available;
    // }
    // if (n == 0) return n;

    size_t here = m_size - m_writePosition;
    if (here >= n) 
    {
		memcpy(m_buffer + m_writePosition, source, n * sizeof(T));
    } 
    else 
    {
		memcpy(m_buffer + m_writePosition, source, here * sizeof(T));
		memcpy(m_buffer, source + here, (n - here) * sizeof(T));
    }

    m_writePosition = (m_writePosition + n) % m_size;

    return n;
}

template <typename T>
size_t
RingBuffer<T>::zero(size_t n)
{
    size_t available = getWriteSpace();
    if (n > available) 
    {
		n = available;
    }
    if (n == 0) return n;

    size_t here = m_size - m_writePosition;
    if (here >= n)
    {
		memset(m_buffer + m_writePosition, 0, n * sizeof(T));
    } 
    else
    {
		memset(m_buffer + m_writePosition, 0, here * sizeof(T));
		memset(m_buffer, 0, (n - here) * sizeof(T));
    }

    m_writePosition = (m_writePosition + n) % m_size;
    return n;
}

#endif // _RINGBUFFER_H_
