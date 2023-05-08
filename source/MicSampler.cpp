#include "MicSampler.h"

MicSampler::MicSampler(DataSource &s, MicroBit *ubit) : source(s)
{
    source.connect(*this);
    DMESG("SAMPLER ONLINE");
    this->ubit = ubit;
}

MicSampler::~MicSampler()
{
    this->source.disconnect();
}

int MicSampler::pullRequest()
{
    if (!active)
    {
        return DEVICE_OK;
    }
    buffer = source.pull();
    time = ubit->systemTime();
    int16_t *data = (int16_t *)&buffer[0];
    for (int around = 0; around < 1; around++)
    {
        f->clearSamples();
        for (int i = 0; i < WINDOW_SIZE; i++)
        {
            f->addSample((int8_t)*data);
            data++;
        }
        bool result = f->processReal();
        if (result)
        {
            this->stop();
        }
    }
    return DEVICE_OK;
}
