#include "MicroBit.h"
#include "fft.h"

FFT::FFT() {}

void FFT::processComplex()
{
    int SampleNumber = DFTOutput.size();
    DMESG("SAMPLE NUMBER: %d", SampleNumber);
    cfg = kiss_fft_alloc(DFTOutput.size(), 0, NULL, NULL);
    kiss_fft_cpx in[SampleNumber], out[SampleNumber];
    for (int i = 0; i < SampleNumber; i++)
    {
        in[i].r = DFTOutput.at(i).real();
        in[i].i = DFTOutput.at(i).imag();
    }

    kiss_fft(cfg, in, out);

    // for (int i = 0; i < SampleNumber; i++)
    // PRINTFIVEFLOAT(i, in[i].r, in[i].i, out[i].r, out[i].i);
    free(cfg);
}

bool FFT::processReal()
{
    // DMESG("SAMPLE NUMBER: %d", sampleNumber);
    cfgr = kiss_fftr_alloc(sampleNumber, 0, NULL, NULL);
    kiss_fft_cpx out[sampleNumber];
    kiss_fftr(cfgr, this->FFTInput, out);
    // DMESG("PROCESSED");

    for (int i = 0; i < sampleNumber; i++)
    {
        FFTOutput[i] = std::complex<double>(out[i].r, out[i].i);
        // FFTOutput.push_back(std::complex<double>(out[i].r, out[i].i));
    }
    // PRINTFOURFLOAT(i, inr[i], out[i].r, out[i].i);

    free(cfgr);
    double mag[sampleNumber / 2];
    // DMESG("START OF NEW LOOP");
    for (int i = 0; i < sampleNumber / 2; i++)
    {
        std::complex<double> val = this->FFTOutput[i];
        mag[i] = (val.real() * val.real()) + (val.imag() * val.imag());
        // PRINTFLOAT(mag[i]);
    }
    // DMESG("END OF NEW LOOP");
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
    // PRINTFLOATMSG("MAX", max);
    // DMESG("INDEX: %d", index);
    // PRINTCOMPLEX(FFTOutput[index].real(), FFTOutput[index].imag());

    // DMESG("PRINTING ALL FREQS:");
    // for (int i = 0; i < sampleNumber / 2; i++)
    // {
    //     PRINTFLOAT((MIC_SAMPLE_RATE / sampleNumber) * (i));
    // }
    float rate = (MIC_SAMPLE_RATE / sampleNumber);
    double freq = (rate) * (index + 1);
    int ind = 2700 / rate;
    // PRINTFLOATMSG("IND", ind);
    // DMESG("int IND: ", (int)ind);
    // PRINTFLOATMSG("FREQUENCY", freq);
    PRINTFLOATMSG("FREQUENCY/2", (int)(freq / 2));
    // PRINTFLOATMSG("FREQ==========================", freq);
    // PRINTFLOAT((rate) * (ind));
    // PRINTFLOAT(mag[ind]);
    // PRINTCOMPLEX(FFTOutput[63].real(), FFTOutput[63].imag());
    if (freq > TRANSMIT_FREQUENCY - 100 && freq < TRANSMIT_FREQUENCY + 100)
    {
        DMESG("IN RANGE");
        PRINTFLOATMSG("FREQUENCY", freq);
        PRINTFLOAT(mag[ind]);
        return true;
    }
    if (mag[ind] > 7500000)
    {
        DMESG("ABOVE LEVEL CAP");
        return true;
    }
    if ((freq / 2) > TRANSMIT_FREQUENCY - 100 && (freq / 2) < TRANSMIT_FREQUENCY + 100)
    {
        DMESG("HALVE IN RANGE");
        PRINTFLOATMSG("FREQUENCY", freq);
        PRINTFLOATMSG("FREQUENCY/2", (int)(freq / 2));
        PRINTFLOAT(mag[ind]);
        return true;
    }
    // double freqWithoutPlusOne = (MIC_SAMPLE_RATE / sampleNumber) * (index);
    // PRINTFLOATMSG("FREQUENCY2", freqWithoutPlusOne);
    return false;
}
