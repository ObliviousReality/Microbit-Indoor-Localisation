#include "kiss/kiss_fftr.h"
#include "global.h"

#ifndef FFT_H
#define FFT_H

class FFT
{
public:
    FFT();
    bool processReal();
    void addSample(float s) { this->FFTInput[sampleNumber++] = s; }
    void clearSamples() { this->sampleNumber = 0; }
    int getSampleNumber() { return this->sampleNumber; }

private:
    int sampleNumber = 0;

    kiss_fft_scalar FFTInput[WINDOW_SIZE];

    kiss_fftr_cfg cfgr;
};

#endif