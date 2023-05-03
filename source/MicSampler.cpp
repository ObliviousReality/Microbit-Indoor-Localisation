#include "MicSampler.h"

MicSampler::MicSampler(DataSource &s) : source(s)
{
    source.connect(*this);
    buf = (int16_t *)malloc(sizeof(int16_t) * windowSize * 2);
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

    int16_t *data = (int16_t *)&b[0];

    int samples = b.length() / 2;

    for (int i = 0; i < samples; i++)
    {
        if (source.getFormat() == DATASTREAM_FORMAT_8BIT_SIGNED)
        {
            workTotal += abs((int8_t)*data);
            if (abs((int8_t)*data) > workMax)
            {
                workMax = abs((int8_t)*data);
            }
            buf[windowPos] = abs((int8_t)*data);
            // DMESG("DATA: %d", (int8_t)*data);
        }
        else
        {
            workTotal += abs(*data);
            if (abs(*data) > workMax)
            {
                workMax = abs(*data);
            }
            buf[windowPos] = abs(*data);
            // DMESG("DATA: %d", *data);
        }

        windowPos++;
        if (windowPos == windowSize)
        {
            // DMESG("workTotal: %d", workTotal);
            // DMESG("workMax: %d", workMax);
            level = workTotal / windowSize;
            if (source.getFormat() == DATASTREAM_FORMAT_8BIT_SIGNED)
            {
                level = level * 8;
            }
            max = workMax;
            total = workTotal;
            // DMESG("%s:\t%d\t| %s:\t%d\t| %s:\t%d", "TOTAL", workTotal, "MAX", workMax, "AVERAGE",
            //   level);
            workTotal = 0;
            workMax = -10000;
            windowPos = 0;
        }
        data++;
    }

    return DEVICE_OK;
}