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

void MicSampler::addSamples(int start, int end)
{
    f->clearSamples();
    int16_t *data = (int16_t *)&buffer[0];
    for (int i = start; i < end; i++)
    {
        this->f->addSample((int8_t)*data++);
    }
}

void MicSampler::testRanges()
{
    DMESG("TEST RANGES");
    int midpoint = (upperBound - lowerBound) / 2;
    this->addSamples(this->lowerBound, midpoint);
    bool r = this->f->processReal();
    if (!r)
    {
        this->addSamples(midpoint + 1, upperBound);
        r = this->f->processReal();
        if (!r)
        {
            DMESG("THIS SHOULDN'T HAPPEN");
            ubit->panic(634);
            return;
        }
        this->lowerBound = midpoint + 1;
        return;
    }
    this->upperBound = midpoint;
    return;
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
    this->addSamples(0, WINDOW_SIZE);
    long bTime = ubit->systemTime();
    bool result = f->processReal();
    long eTime = ubit->systemTime();
    PRINTFLOATMSG("FFT TIME", eTime - bTime);
    if (result)
    {
        // Binary Search Time.

        this->lowerBound = 0;
        this->upperBound = WINDOW_SIZE;
        for (int i = 1; i < 5; i++)
        {
            this->testRanges();
            DMESG("LOWER BOUND: %d", lowerBound);
            DMESG("UPPER BOUND: %d", upperBound);
        }

        DMESG("STOPPING");
        fiber_sleep(1);
        this->outcome = true;
    }
    // }
    return DEVICE_OK;
}
