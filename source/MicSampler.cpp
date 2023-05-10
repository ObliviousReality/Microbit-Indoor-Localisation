#include "MicSampler.h"
#include <time.h>

MicSampler::MicSampler(DataSource &s, MicroBit *ubit) : source(s)
{
    source.connect(*this);
    DMESG("SAMPLER ONLINE");
    PRINTFLOATMSG("SAMPLE RATE OF SAMPLER", source.getSampleRate());
    this->ubit = ubit;
}

MicSampler::~MicSampler()
{
    this->source.disconnect();
}

int MicSampler::pullRequest()
{
    PRINTFLOATMSG("PR AT", ubit->systemTime()); // roughly every 23 ms but does slightly vary.
    if (!active || outcome)
    {
        return DEVICE_OK;
    }
    buffer = source.pull();
    time = ubit->systemTime();
    aRecv = clock();
    int16_t *data = (int16_t *)&buffer[0];
    // for (int around = 0; around < 1; around++)
    // {
    f->clearSamples();
    for (int i = 0; i < WINDOW_SIZE; i++)
    {
        f->addSample((int8_t)*data);
        data++;
    }
    long bTime = ubit->systemTime();
    bool result = f->processReal();
    long eTime = ubit->systemTime();
    PRINTFLOATMSG("FFT TIME", eTime - bTime);
    if (result)
    {
        DMESG("STOPPING");
        fiber_sleep(1);
        this->outcome = true;
    }
    // }
    return DEVICE_OK;
}
