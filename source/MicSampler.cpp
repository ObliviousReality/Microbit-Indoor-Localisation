#include "MicSampler.h"

MicSampler::MicSampler(DataSource &s) : source(s)
{
    source.connect(*this);
    DMESG("SAMPLER ONLINE");
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
    ManagedBuffer b = source.pull();
    buffer = b;
    return DEVICE_OK;
}