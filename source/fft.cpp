#include "MicroBit.h"
#include "fft.h"

FFT::FFT() {}

bool FFT::processReal(int MagThreshold)
{
    DMESG("SN:%d", sampleNumber);
    kiss_fftr_cfg cfgr = kiss_fftr_alloc(sampleNumber, 0, NULL, NULL);
    kiss_fft_cpx out[sampleNumber];
    kiss_fftr(cfgr, this->FFTInput, out);
    free(cfgr);

    float mag[sampleNumber / 2];
    for (int i = 0; i < sampleNumber / 2; i++)
    {
        kiss_fft_cpx val = out[i];
        mag[i] = (((val.r * val.r) + (val.i * val.i)));
    }

    double max = 0;
    int index = 0;
    float rate = (MIC_SAMPLE_RATE / sampleNumber);
    for (int i = 1; i < sampleNumber / 2; i++)
    {
        if (mag[i] > 0)
            PRINTTWOFLOATMSG("FREQ/MAG", (int)(rate * i), mag[i]);
        // DMESG("FREQ: %d, MAG: %d", (int)(rate * i), mag[i]);
        if (mag[i] > max)
        {
            max = mag[i];
            index = i;
        }
    }

    if (max == 0)
    {
        // DMESG("NO DATA");
        return false;
    }

    double freq = (rate) * (index + 1);
    this->frequency = freq;
    // DMESG("BIN WIDTH: %d", (int)(freq - (rate * (index))));
    int ind = 2700 / rate;
    this->magnitude = mag[ind];
    int ind2 = 5400 / rate;
    this->magnitude5400 = mag[ind2];
    // if ((freq / 2) > TRANSMIT_FREQUENCY - 100 && (freq / 2) < TRANSMIT_FREQUENCY + 100)
    {
        if (mag[ind] < MAG_THRESHOLD || mag[ind2] < MAG_THRESHOLD)
        {
            // DMESG("MAG TOO LOW");
            // if (mag[ind2] < MAG_THRESHOLD)
            // {
            //     DMESG("but mag 2 isn't...");
            // }
            return false;
        }
        return true;
    }
    return false;
}
