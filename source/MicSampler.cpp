#include "MicSampler.h"
#include <time.h>

MicSampler::MicSampler(DataSource &s, MicroBit *ubit) : source(s)
{
    source.connect(*this);
    DMESG("SAMPLER ONLINE");
    PRINTFLOATMSG("SAMPLE RATE OF SAMPLER", source.getSampleRate());
    this->ubit = ubit;

    // this->buffers = (ManagedBuffer **)malloc(sizeof(ManagedBuffer *) * BUFFER_BUFFER);
    // this->times = (long *)malloc(sizeof(long) * BUFFER_BUFFER);
    // this->found = (bool *)malloc(sizeof(bool) * BUFFER_BUFFER);
    // this->mags = (int *)malloc(sizeof(int) * BUFFER_BUFFER);
    // this->subFound = (bool **)malloc(sizeof(bool *) * BUFFER_BUFFER);
    // this->subMags = (int **)malloc(sizeof(int *) * BUFFER_BUFFER);
}

MicSampler::~MicSampler()
{
    this->source.disconnect();
}

void MicSampler::addSamples(int start, int end, ManagedBuffer b)
{
    f->clearSamples();
    int16_t *data = (int16_t *)&b[0];
    data = data + start;
    DMESG("ADDING SAMPLE 1: %d", (int8_t)*data);
    for (int i = start; i < end; i++)
    {
        this->f->addSample((int8_t)*data++);
    }
    DMESG("ADDING SAMPLE N: %d", (int8_t)*data);
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

void MicSampler::slidingWindow()
{
    DMESG("SLIDING WINDOW");
    fiber_sleep(1);
    // int numberOfSplits = 16;
    // int frameSize = (WINDOW_SIZE / numberOfSplits);
    // int *magnitudes = (int *)malloc(sizeof(int) * numberOfSplits);
    // int mCount = 0;
    // DMESG("SW LOOP START");
    // fiber_sleep(1);
    // for (int i = 0; i < WINDOW_SIZE; i += frameSize)
    // {
    //     ubit->display.print("L");
    //     DMESG("WINDOW RANGE: %d -> %d", i, i + frameSize);
    //     fiber_sleep(10);
    //     this->addSamples(i, i + frameSize);
    //     bool r = this->f->processReal();
    //     // if (r)
    //     magnitudes[mCount++] = this->f->getMag();
    //     // else
    //     // magnitudes[mCount++] = 0;
    // }

    // for (int i = 0; i < numberOfSplits; i++)
    // {
    //     DMESG("MAGNITUDE AT POS %d: %d", i, magnitudes[i]);
    // }
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
    PRINTFLOATMSG("PR AT", ubit->systemTime() - 23); // roughly every 23 ms but does slightly vary.
    fiber_sleep(1);
    time = ubit->systemTime() - 23; // Sample length is 23ms, so sample began 23ms earlier than PR.
    buffer = source.pull();
    aRecv = clock();
    buffers[bufCounter] = new AudioBuffer(buffer, time);
    bufCounter++;
    /*this->addSamples(0, WINDOW_SIZE);
    long bTime = ubit->systemTime();

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    bool result = f->processReal();

    uint32_t cycleCount = DWT->CYCCNT;
    DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk;
    DMESG("cycle count: %d", cycleCount);

    long eTime = ubit->systemTime();
    PRINTFLOATMSG("FFT TIME", eTime - bTime);
    if (result)
    {
        this->active = false;
        ubit->display.print("B");
        // Binary Search Time.

        // this->lowerBound = 0;
        // this->upperBound = WINDOW_SIZE;
        // for (int i = 1; i < 4; i++)
        // {
        //     DMESG("LOOP %d: ", i);
        //     this->binaryChop();
        //     DMESG("LOWER BOUND: %d", lowerBound);
        //     DMESG("UPPER BOUND: %d", upperBound);
        // }

        this->slidingWindow();

        DMESG("STOPPING");
        fiber_sleep(1);
        this->outcome = true;
    }
    else
    {
        ubit->display.print("F");
    }
    // }
    */
    return DEVICE_OK;
}

void MicSampler::processResult()
{
    for (int i = 0; i < BUFFER_BUFFER; i++)
    {
        this->addSamples(0, WINDOW_SIZE, buffers[i]->buffer);
    }
}