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

    for (int i = 0; i < SampleNumber; i++)
        PRINTFIVEFLOAT(i, in[i].r, in[i].i, out[i].r, out[i].i);
    free(cfg);
}

void FFT::processReal()
{
    int SampleNumber = DFTOutput.size();
    DMESG("SAMPLE NUMBER: %d", SampleNumber);
    cfgr = kiss_fftr_alloc(DFTOutput.size(), 0, NULL, NULL);
    kiss_fft_scalar inr[SampleNumber];
    kiss_fft_cpx out[SampleNumber];
    for (int i = 0; i < SampleNumber; i++)
    {
        inr[i] = DFTInput.at(i);
    }

    kiss_fftr(cfgr, inr, out);

    for (int i = 0; i < SampleNumber; i++)
        PRINTFOURFLOAT(i, inr[i], out[i].r, out[i].i);

    free(cfgr);
}