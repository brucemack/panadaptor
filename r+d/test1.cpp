#include <stdio.h>
#include <math.h>

// Morse array.  MSB->LSB starting from most significant 1
const unsigned long MORSE_CODES[] = {
	0xb8,	// A (65) code=10111000
	0xea8,	// B (66) code=111010101000
	0x3ae8,	// C (67) code=11101011101000
	0x3a8,	// D (68) code=1110101000
	0x8,	// E (69) code=1000
	0xae8,	// F (70) code=101011101000
	0xee8,	// G (71) code=111011101000
	0x2a8,	// H (72) code=1010101000
	0x28,	// I (73) code=101000
	0xbbb8,	// J (74) code=1011101110111000
	0xeb8,	// K (75) code=111010111000
	0xba8,	// L (76) code=101110101000
	0x3b8,	// M (77) code=1110111000
	0xe8,	// N (78) code=11101000
	0x3bb8,	// O (79) code=11101110111000
	0x2ee8,	// P (80) code=10111011101000
	0xeeb8,	// Q (81) code=1110111010111000
	0x2e8,	// R (82) code=1011101000
	0xa8,	// S (83) code=10101000
	0x38,	// T (84) code=111000
	0x2b8,	// U (85) code=1010111000
	0xab8,	// V (86) code=101010111000
	0xbb8,	// W (87) code=101110111000
	0x3ab8,	// X (88) code=11101010111000
	0xebb8,	// Y (89) code=1110101110111000
	0x3ba8,	// Z (90) code=11101110101000
	0x3bbbb8,	// 0 (48) code=1110111011101110111000
	0xbbbb8,	// 1 (49) code=10111011101110111000
	0x2bbb8,	// 2 (50) code=101011101110111000
	0xabb8,	// 3 (51) code=1010101110111000
	0x2ab8,	// 4 (52) code=10101010111000
	0xaa8,	// 5 (53) code=101010101000
	0x3aa8,	// 6 (54) code=11101010101000
	0xeea8,	// 7 (55) code=1110111010101000
	0x3bba8,	// 8 (56) code=111011101110101000
	0xeeee8,	// 9 (57) code=11101110111011101000
	0x2bba8,	// ? (63) code=101011101110101000
	0xbaeb8,	// . (46) code=10111010111010111000
	0x3babb8,	// , (44) code=1110111010101110111000
	0xeae8,	// / (47) code=1110101011101000
	0x3aab8,	// - (45) code=111010101010111000
};

// English codes (in same sequence)
const char ENGLISH_CODES[] = {
  'A',
  'B',
  'C',
  'D',
  'E',
  'F',
  'G',
  'H',
  'I',
  'J',
  'K',
  'L',
  'M',
  'N',
  'O',
  'P',
  'Q',
  'R',
  'S',
  'T',
  'U',
  'V',
  'W',
  'X',
  'Y',
  'Z',
  '0',
  '1',
  '2',
  '3',
  '4',
  '5',
  '6',
  '7',
  '8',
  '9',
  '?',
  '.',
  ',',
  '/',
  '-',
};

// ===========================================================================
// This class is used to convert a binary-encoded CW symbol into a set of
// baseband samples at the appropriate speed and resolution.  This is
// useful for sending CW symbols as well as generating the baseband pattern
// that can be compared to a.

class CWData {

  // A constant used for masking off the MSB of a code
  unsigned long msbMask;
  // The time between samples in milliseconds
  unsigned int sampleInterval;
  // The configured Morse code word-per-minute rate to use
  unsigned int wpm;
  // This is the number of dit spaces per sample, based on the current WPM
  float ditsPerSample;
  // The code that is actively being sampled
  unsigned long workingCode;
  // The number of elements (dit spaces) remaining in the code being sampled
  unsigned int workingCodeLen;
  // These are used for tracking progress across the character
  unsigned int samplesRemaining;
  // We use float to avoid losing precision during the traversal of the code
  float fractionalPtr;
  unsigned int integerPtr;

  public:

    CWData();

    void reset(unsigned int sampleIntervalMs,
      unsigned int wpm,unsigned long code);
    unsigned int getSamplesRemaining();
    int getSample();
};

CWData::CWData() {
  // NOTE: We do this because the datatype size can differ across platforms
  unsigned int initialSize = sizeof(unsigned long) * 8;
  unsigned long msbMask = 1;
  for (unsigned int i = 0; i < initialSize - 1; i++) {
    msbMask <<= 1;
  }
  this->msbMask = msbMask;
}

void CWData::reset(unsigned int sampleInterval,
  unsigned int wpm,unsigned long code) {
  // NOTE: We do this because the datatype size can differ across platforms
  unsigned int initialSize = sizeof(unsigned long) * 8;
  // Shift out the leading zeros, keeping track of what remains
  workingCode = code;
  workingCodeLen = initialSize;
  while ((workingCode & msbMask) == 0 && (workingCodeLen > 0)) {
    workingCode <<= 1;
    workingCodeLen--;
  }

  // Do math related to code speed
  sampleInterval = sampleInterval;
  wpm = wpm;
  // Compute the dit time in ms
  unsigned int ditTime = 1200 / wpm;
  // Compute how many (fractional) dits each sample represents
  float samplesPerDit = (float)ditTime / (float)sampleInterval;
  ditsPerSample = 1.0 / samplesPerDit;
  fractionalPtr = 0;
  integerPtr = 0;
  // Calculate the total number of samples required (in integer terms)
  samplesRemaining = (float)workingCodeLen * samplesPerDit;
}

unsigned int CWData::getSamplesRemaining() {
  return samplesRemaining;
}

int CWData::getSample() {
  // Check if we're off the end of the character
  if (samplesRemaining == 0) {
    return 0;
  }
  int result = 0;
  // Figure out what our sample looks like (0 or 1)
  if (workingCode & msbMask) {
    result = 1;
  }
  fractionalPtr += ditsPerSample;
  // Check to see if we've rolled over to a new integer dit
  if (integerPtr != (unsigned int)fractionalPtr) {
    integerPtr += 1;
    workingCode <<= 1;
  }
  samplesRemaining -= 1;
  return result;
}

int main(int argc,const char** argv) {

  // F
  CWData data0;
  data0.reset(8,12,MORSE_CODES[0]);
  unsigned int count0 = data0.getSamplesRemaining();

  // G
  CWData data1;
  data1.reset(8,12,MORSE_CODES[0]);
  unsigned int count1 = data1.getSamplesRemaining();

  int diffSum = 0;
  for (unsigned int i = 0; i < count0; i++) {
    int sample0 = data0.getSample();
    int sample1 = data1.getSample();
    printf("%u  %d  %d\n",i,sample0,sample1);
    int diff = (sample0 - sample1) * (sample0 - sample1);
    diffSum += diff;
  }

  float error = sqrt(diffSum) / (float)count0;
  printf("Error %f\n",error);

  return 0;
}
