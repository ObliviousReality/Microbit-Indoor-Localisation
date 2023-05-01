#include "MicroBit.h"
#include <complex.h>
#include <vector>
#include "kiss/kiss_fft.h"

#ifndef FFT_H
#define FFT_H

class FFT
{
public:
    FFT();
    void process();
    void DFT()
    {
        int len = input.size();
        DMESG("LEN: %d", len);
        float r, i;
        for (int k = 0; k < len; k++)
        {
            r = 0;
            i = 0;
            for (int n = 0; n < len; n++)
            {
                double in = input.at(n);
                // DMESG("INPUT: %d", (int)in);
                r = (r + in * cos(2 * PI * k * n / len));
                i = (i - in * sin(2 * PI * k * n / len));
            }
            // real.push_back(r);
            // imaginary.push_back(i);
            DMESG("REAL: %d", (int)r);
            DMESG("IMAG: %d", (int)i);
            output.push_back(std::complex<double>(r, i));
        }
    }

    void setInput(std::vector<double> din) { this->input = din; }
    void addSample(float s)
    {
        this->input.push_back(s);
        DMESG("ADDING %d", (int)s);
    }
    std::vector<std::complex<double>> *getOutput() { return &output; }

private:
    std::vector<double> input;
    // std::vector<double> real;
    // std::vector<double> imaginary;
    std::vector<std::complex<double>> output;
};

#endif