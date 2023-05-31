#include "kiss/kiss_fftr.h"
#include "global.h"

#ifndef FFT_H
#define FFT_H

class FFT
{
public:
    FFT();
    bool processReal();
    void addSample(int8_t s) { this->FFTInput[sampleNumber++] = s; }
    void clearSamples() { this->sampleNumber = 0; }
    int getSampleNumber() { return this->sampleNumber; }
    int getMag() { return this->magnitude; }
    int getMagTwo() { return this->magnitude5400; }

private:
    int sampleNumber = 0;

    kiss_fft_scalar FFTInput[WINDOW_SIZE];

    int magnitude = 0;
    int magnitude5400 = 0;
};

#endif
