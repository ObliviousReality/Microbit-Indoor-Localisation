#include "MicroBit.h"
#include <complex>
#include <vector>
#include "kiss/kiss_fft.h"
#include "kiss/kiss_fftr.h"
#include "global.h"

#ifndef FFT_H
#define FFT_H

class FFT
{
public:
    FFT();
    void processComplex();
    void processReal();
    void DFT()
    {
        int len = sampleNumber;
        DMESG("LEN: %d", len);
        float r, i;
        for (int k = 0; k < len; k++)
        {
            r = 0;
            i = 0;
            for (int n = 0; n < len; n++)
            {
                double in = DFTInput.at(n);
                // DMESG("INPUT: %d", (int)in);
                r = (r + in * cos(2 * PI * k * n / len));
                i = (i - in * sin(2 * PI * k * n / len));
            }
            // real.push_back(r);
            // imaginary.push_back(i);
            // PRINTFLOATMSG("REAL", r);
            // PRINTFLOATMSG("IMAG", i);
            DFTOutput.push_back(std::complex<double>(r, i));
        }
    }
    void addSample(float s)
    {
        this->DFTInput.push_back(s);
        // PRINTFLOATMSG("ADDING", s);
        PRINTFLOAT(s);
        this->sampleNumber++;
    }
    void clearSamples()
    {
        this->DFTInput.clear();
        this->DFTOutput.clear();
        this->FFTOutput.clear();
        this->sampleNumber = 0;
    }
    std::vector<std::complex<double>> *getDFTOutput() { return &DFTOutput; }

    int getSampleNumber() { return this->sampleNumber; }

private:
    int sampleNumber = 0;

    std::vector<double> DFTInput;
    // std::vector<double> real;
    // std::vector<double> imaginary;
    std::vector<std::complex<double>> DFTOutput;
    std::vector<std::complex<double>> FFTOutput;
    kiss_fftr_cfg cfgr;
    kiss_fft_cfg cfg;
};

#endif