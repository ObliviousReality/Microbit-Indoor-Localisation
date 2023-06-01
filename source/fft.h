#include "kiss/kiss_fftr.h"
#include "global.h"

#ifndef FFT_H
#define FFT_H

class FFT
{
public:
    FFT();
    bool processReal(int MagThreshold = MAG_THRESHOLD);
    void addSample(int8_t s) { this->FFTInput[sampleNumber++] = s; }
    void clearSamples() { this->sampleNumber = 0; }
    int getSampleNumber() { return this->sampleNumber; }
    int getMag() { return this->magnitude; }
    int getMagTwo() { return this->magnitude5400; }

    int frequency = 0;

private:
    int sampleNumber = 0;

    kiss_fft_scalar FFTInput[WINDOW_SIZE];

    float magnitude = 0;
    int magnitude5400 = 0;
};

#endif
