#ifndef _CircularBuffer_h
#define _CircularBuffer_h

class FIFOBuffer {

public:

  FIFOBuffer(unsigned char* buf,unsigned int bufLen);

  void clear();
  void write(unsigned char c);
  unsigned int available() const;
  unsigned char read();
  void saveReadPoint();
  void returnToReadPoint();

private:

  unsigned char* buf;
  // Pointer to the byte just beyond the end of the buffer. Used to detect
  // end-of-buffer (wrap) situations.
  const unsigned char* bufEnd;
  // The location of the next write
  unsigned char* writePtr;
  // The location of the next read
  unsigned char* readPtr;
  // Used for tracking when the buffer has filled up
  bool isFull;
  // Used to save the read state
  unsigned char* savedReadPtr;
  bool savedIsFull;
};

#endif
