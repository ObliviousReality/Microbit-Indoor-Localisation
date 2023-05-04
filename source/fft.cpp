#include "MicroBit.h"
#include "fft.h"

FFT::FFT() {}

bool FFT::processReal()
{
    cfgr = kiss_fftr_alloc(sampleNumber, 0, NULL, NULL);
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
    PRINTFLOATMSG("FREQUENCY/2", (int)(freq / 2));
    if (freq > TRANSMIT_FREQUENCY - 100 && freq < TRANSMIT_FREQUENCY + 100)
    {
        DMESG("IN RANGE");
        PRINTFLOATMSG("FREQUENCY", freq);
        PRINTFLOAT(mag[ind]);
        return true;
    }
    if ((freq / 2) > TRANSMIT_FREQUENCY - 100 && (freq / 2) < TRANSMIT_FREQUENCY + 100)
    {
        DMESG("HALVE IN RANGE");
        PRINTFLOATMSG("FREQUENCY", freq);
        PRINTFLOATMSG("FREQUENCY/2", (int)(freq / 2));
        PRINTFLOATMSG("MAG:", mag[ind]);
        return true;
    }
    return false;
}
