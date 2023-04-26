#include "MicSampler.h"

MicSampler::MicSampler(DataSource &s) : source(s)
{
    source.connect(*this);
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

    int16_t *data = (int16_t *)&b[0];

    int samples = b.length() / 2;

    for (int i = 0; i < samples; i++)
    {
        if (source.getFormat() == DATASTREAM_FORMAT_8BIT_SIGNED)
        {
            total += abs((int8_t)*data);
        }
        else
        {
            total += abs(*data);
        }

        windowPos++;
        if (windowPos == windowSize)
        {
            level = total / windowSize;
            total = 0;
            windowPos = 0;
            if (source.getFormat() == DATASTREAM_FORMAT_8BIT_SIGNED)
            {
                level = level * 8;
            }
        }
        data++;
    }

    return DEVICE_OK;
}