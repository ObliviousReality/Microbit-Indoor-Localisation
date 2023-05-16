#include "MicSampler.h"
#include <time.h>
#include "AudioTimer.h"

MicSampler::MicSampler(DataSource &s, MicroBit *ubit) : source(s)
{
    source.connect(*this);
    DMESG("SAMPLER ONLINE");
    PRINTFLOATMSG("SAMPLE RATE OF SAMPLER", source.getSampleRate());
    this->ubit = ubit;
}

MicSampler::~MicSampler()
{
    DMESG("DECON");
    fiber_sleep(1);
    this->source.disconnect();
}

bool MicSampler::processFFT()
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    bool result = f->processReal();

    uint32_t cycleCount = DWT->CYCCNT;
    DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk;
    // DMESG("cycle count: %d", cycleCount);
    return result;
}

void MicSampler::addSamples(int start, int end, ManagedBuffer b)
{
    f->clearSamples();
    int16_t *data = (int16_t *)&b[0];
    data = data + start;
    // DMESG("ADDING SAMPLE 1: %d", (int8_t)*data);
    int i;
    for (i = start; i < end; i++)
    {
        this->f->addSample((int8_t)*data++);
    }
    // DMESG("ADDING SAMPLE %d: %d", i, (int8_t)*data);
}

void MicSampler::binaryChop()
{
    DMESG("TEST RANGES");
    // int midpoint = (upperBound - lowerBound) / 2;
    // this->addSamples(this->lowerBound, midpoint);
    // bool r1 = this->f->processReal();
    // int m1 = this->f->getMag();
    // this->addSamples(midpoint + 1, upperBound);
    // bool r2 = this->f->processReal();
    // int m2 = this->f->getMag();
    // if (!r1 && !r2)
    // {
    //     DMESG("THIS SHOULDN'T HAPPEN");
    //     fiber_sleep(1000);
    //     ubit->panic(634);
    //     return;
    // }
    // if (r1 && r2)
    // {
    //     if (m1 > m2)
    //         r2 = false;
    //     else
    //         r1 = false;
    // }
    // if (r1)
    // {
    //     this->upperBound = midpoint;
    //     return;
    // }
    // this->lowerBound = midpoint + 1;
    return;
}

int MicSampler::slidingWindow(ManagedBuffer b)
{
    // ubit->display.print("W");
    DMESG("SLIDING WINDOW");
    int frameSize = (WINDOW_SIZE / SPLIT_NUMBER);
    int *magnitudes = (int *)malloc(sizeof(int) * SPLIT_NUMBER);
    bool *founds = (bool *)malloc(sizeof(bool) * SPLIT_NUMBER);
    int mCount = 0;
    DMESG("SW LOOP START");
    fiber_sleep(1);
    for (int i = 0; i < WINDOW_SIZE; i += frameSize)
    {
        DMESG("WINDOW RANGE: %d -> %d", i, i + frameSize);
        fiber_sleep(10);
        this->addSamples(i, i + frameSize, b);
        bool r = this->f->processReal();
        // if (r)
        founds[mCount] = r;
        magnitudes[mCount++] = this->f->getMag();
        // else
        // magnitudes[mCount++] = 0;
    }
    for (int i = 0; i < SPLIT_NUMBER; i++)
    {
        DMESG("MAGNITUDE AT POS %d: %d", i, magnitudes[i]);
        // ubit->display.clear();
        // ubit->sleep(1000);
        // ubit->display.print(magnitudes[i]);
        // ubit->sleep(1000);
    }

    int ctr = 0;
    int firstTrueIndex;
    int lastTrueIndex;
    frontIndexFlag = true;
    while (!founds[ctr])
    {
        ctr++;
        if (ctr > SPLIT_NUMBER)
            return -1;
    }

    firstTrueIndex = ctr;
    if (firstTrueIndex == 0)
    {
        while (founds[ctr])
        {
            ctr++;
            if (ctr > SPLIT_NUMBER)
                return firstTrueIndex;
        }
        frontIndexFlag = false;
        lastTrueIndex = ctr;
    }
    fiber_sleep(10);
    return firstTrueIndex;
}

int MicSampler::pullRequest()
{
    if (!active)
    {
        return DEVICE_OK;
    }
    if (bufCounter == BUFFER_BUFFER)
    {
        DMESG("STOPPING, BUFFER FILLED");
        this->stop();
        fiber_sleep(10);
        return DEVICE_OK;
    }
    PRINTFLOATMSG("PR AT", AudioTimer::audioTime -
                               SAMPLE_LENGTH_US); // roughly every 23 ms but does slightly vary.
    time = AudioTimer::audioTime - SAMPLE_LENGTH_US;
    buffer = source.pull();
    aRecv = clock();
    buffers[bufCounter] = new AudioBuffer(bufCounter, buffer, time);
    bufCounter++;
    return DEVICE_OK;
}

bool MicSampler::processResult()
{
    bool anyFound = false;
    for (int i = 0; i < BUFFER_BUFFER; i++)
    {
        this->addSamples(0, WINDOW_SIZE, buffers[i]->buffer);
        bool r = this->f->processReal();
        buffers[i]->found = r;
        buffers[i]->mag = f->getMag();
        if (r)
        {
            anyFound = true;
        }
        PRINTBUFFER(buffers[i]);
    }

    for (int i = 0; i < BUFFER_BUFFER; i++)
    {
        if (buffers[i]->found)
        {
            DMESG("%d:", i);
            int index = slidingWindow(buffers[i]->buffer);
            if (index >= 0)
            {
                if (frontIndexFlag)
                {
                    time = buffers[i]->time + (SAMPLE_LENGTH_MS * (index / SPLIT_NUMBER));
                    PRINTFLOATMSG("TIME", time);
                    // ubit->display.print(time);
                }
                else
                {
                    time = buffers[i]->time + (SAMPLE_LENGTH_MS * (index / SPLIT_NUMBER)) - 20;
                    PRINTFLOATMSG("TIME", time);
                }
            }
            fiber_sleep(1);
        }
    }

    if (!anyFound)
    {
        DMESG("NOT FOUND FREQUENCY");
        fiber_sleep(10);
        return false;
    }
    ubit->display.print("T");
    DMESG("RETURN TRUE");
    fiber_sleep(1);
    return true;
}