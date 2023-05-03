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
    //     PRINTFIVEFLOAT(i, in[i].r, in[i].i, out[i].r, out[i].i);
    free(cfg);
}

void FFT::processReal()
{
    // DMESG("SAMPLE NUMBER: %d", sampleNumber);
    cfgr = kiss_fftr_alloc(sampleNumber, 0, NULL, NULL);
    kiss_fft_scalar inr[sampleNumber];
    kiss_fft_cpx out[sampleNumber];
    for (int i = 0; i < sampleNumber; i++)
    {
        inr[i] = DFTInput.at(i);
    }

    kiss_fftr(cfgr, inr, out);
    // DMESG("PROCESSED");

    for (int i = 0; i < sampleNumber; i++)
        FFTOutput.push_back(std::complex<double>(out[i].r, out[i].i));
    //     PRINTFOURFLOAT(i, inr[i], out[i].r, out[i].i);

    free(cfgr);
    double mag[sampleNumber / 2];
    // DMESG("START OF NEW LOOP");
    for (int i = 0; i < sampleNumber / 2; i++)
    {
        std::complex<double> val = this->FFTOutput.at(i);
        mag[i] = (val.real() * val.real()) + (val.imag() * val.imag());
        // PRINTFLOAT(mag[i]);
    }
    // DMESG("END OF NEW LOOP");
    // DMESG("MAG:");
    // for (int i = 0; i < sampleNumber / 2; i++)
    // {
    //     PRINTFLOAT(mag[i]);
    // }
    // DMESG("END OF MAG");
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
    PRINTFLOATMSG("MAX", max);
    DMESG("INDEX: %d", index);
    PRINTCOMPLEX(FFTOutput.at(index).real(), FFTOutput.at(index).imag());

    double freq = (MIC_SAMPLE_RATE / sampleNumber) * (index + 1);
    PRINTFLOATMSG("FREQUENCY", freq);
}
