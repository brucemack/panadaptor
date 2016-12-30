#include "FIFOBuffer.h"
#include <stdio.h>

FIFOBuffer::FIFOBuffer(unsigned char* buf,unsigned int bufLen)
  : buf(buf),
    bufEnd(buf + bufLen),
    isFull(false),
    readPtr(buf),
    writePtr(buf) {
}

void FIFOBuffer::clear() {
  readPtr = buf;
  writePtr = buf;
  isFull = false;
}

void FIFOBuffer::write(unsigned char c) {

  // If we're full then move the read pointer forward to make room for the
  // new entry.
  if (isFull) {
    // Advance the poninter and deal with the wrap
    if (++readPtr == bufEnd) {
      readPtr = buf;
    }
  }

  // Store the new data
  *writePtr = c;

  // Advance the poninter and deal with the wrap
  if (++writePtr == bufEnd) {
    writePtr = buf;
  }

  // If, after being moved forward, the write pointer is overlapped with the
  // read pointer then we just filled up the buffer
  isFull = (writePtr == readPtr);
}

unsigned int FIFOBuffer::available() const {
  if (isFull) {
    return (bufEnd - buf);
  } else {
    if (readPtr <= writePtr) {
      return (writePtr - readPtr);
    } else {
      // Distance to end
      unsigned int toEnd = bufEnd - readPtr;
      // Distance from start
      unsigned int fromStart = writePtr - buf;
      return toEnd + fromStart;
    }
  }
}


void FIFOBuffer::saveReadPoint() {
  savedReadPtr = readPtr;
  savedIsFull = isFull;
}

void FIFOBuffer::returnToReadPoint() {
  readPtr = savedReadPtr;
  isFull = savedIsFull;
}

unsigned char FIFOBuffer::read() {
  // When both pointers are on top of each other then the FIFO is empty
  if (readPtr == writePtr && !isFull) {
    return 0;
  }
  unsigned char result = *readPtr;
  if (++readPtr == bufEnd) {
    readPtr = buf;
  }
  isFull = false;
  return result;
}
