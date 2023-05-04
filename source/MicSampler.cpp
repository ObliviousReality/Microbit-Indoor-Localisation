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
    return DEVICE_OK;
}