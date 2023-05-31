#include "MicroBit.h"
#include "fft.h"

#define MAG_THRESHOLD 20

FFT::FFT() {}

bool FFT::processReal()
{
    kiss_fftr_cfg cfgr = kiss_fftr_alloc(sampleNumber, 0, NULL, NULL);
    kiss_fft_cpx out[sampleNumber];
    kiss_fftr(cfgr, this->FFTInput, out);
    free(cfgr);

    double mag[sampleNumber / 2];
    for (int i = 0; i < sampleNumber / 2; i++)
    {
        kiss_fft_cpx val = out[i];
        mag[i] = (val.r * val.r) + (val.i * val.i);
    }

    double max = 0;
    int index = 0;
    for (int i = 1; i < sampleNumber / 2; i++)
    {
        if (mag[i] > max)
        {
            max = mag[i];
            index = i;
        }
    }

    if (max == 0)
    {
        DMESG("NO DATA");
        return false;
    }

    float rate = (MIC_SAMPLE_RATE / sampleNumber);
    double freq = (rate) * (index + 1);
    int ind = 2700 / rate;
    this->magnitude = mag[ind];
    int ind2 = 5400 / rate;
    this->magnitude5400 = mag[ind2];
    // if ((freq / 2) > TRANSMIT_FREQUENCY - 100 && (freq / 2) < TRANSMIT_FREQUENCY + 100)
    {
        if (mag[ind] < MAG_THRESHOLD)
        {
            DMESG("MAG TOO LOW");
            if (mag[ind2] < MAG_THRESHOLD)
            {
                DMESG("but mag 2 isn't...");
            }
            return false;
        }
        return true;
    }
    return false;
}
