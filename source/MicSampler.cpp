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

int MicSampler::slidingWindowTwo(ManagedBuffer b)
{
    int curMag = 0;
    int prevMag = 0;
    bool curFound = false;
    bool prevFound = true;
    for (int i = 0; i < b.length() - SLIDINGWINDOWSIZE; i++)
    {
        this->addSamples(i, i + SLIDINGWINDOWSIZE, b);
        curFound = this->f->processReal();
        curMag = this->f->getMag();
        if (i != 0)
        {
            if ((curFound && !prevFound))
            {
                // ubit->log.logData("CURFOUND", curFound);
                // ubit->log.logData("PREVFOUND", prevFound);
                // ubit->log.logData("CURMAG", curMag);
                // ubit->log.logData("PREVMAG", prevMag);
                return i;
            }
        }
        prevFound = curFound;
        prevMag = curMag;
    }
    return -1;
}

int MicSampler::slidingWindow(ManagedBuffer b)
{
    // ubit->display.print("W");
    DMESG("SLIDING WINDOW");
    int frameSize = (WINDOW_SIZE / SPLIT_NUMBER);
    int *magnitudes = (int *)malloc(sizeof(int) * SPLIT_NUMBER);
    bool *founds = (bool *)malloc(sizeof(bool) * SPLIT_NUMBER);
    int mCount = 0;
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

    int ctr = 0;
    while (!founds[ctr])
    {
        ctr++;
        if (ctr > SPLIT_NUMBER)
            return -1;
    }
    // fiber_sleep(10);
    return ctr;
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
    if (!ubit->audio.mic->isEnabled())
    {
        DMESG("MIC DISABLED, RESETTING");
        ubit->reset();
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
        bool a = this->f->processReal();
        this->addSamples(WINDOW_SIZE, SAMPLE_SIZE, buffers[i]->buffer);
        bool b = this->f->processReal();
        if (a || b)
        {
            anyFound = true;
        }
    }

    if (!anyFound)
    {
        DMESG("NOT FOUND FREQUENCY");
        fiber_sleep(10);
        return false;
    }

    // int fullData[(SAMPLE_SIZE * 2) + (2 * SLIDINGWINDOWSIZE)];
    ManagedBuffer fullBuffer = ManagedBuffer((SAMPLE_SIZE * 2) + (2 * SLIDINGWINDOWSIZE));
    int tempBufInd = 0;
    int16_t *bufferOneData = (int16_t *)&buffers[1]->buffer[0];
    int16_t *bufferTwoData = (int16_t *)&buffers[2]->buffer[0];
    for (int i = 0; i < (SAMPLE_SIZE * 2) + (2 * SLIDINGWINDOWSIZE); i++)
    {
        if (i < SLIDINGWINDOWSIZE || i > (SAMPLE_SIZE * 2) + SLIDINGWINDOWSIZE)
        {
            // fullData[i] = 0;
            fullBuffer.setByte(i, 0);
        }
        if (i == SLIDINGWINDOWSIZE + SAMPLE_SIZE)
        {
            tempBufInd++;
        }
        if (!tempBufInd)
        {
            // fullData[i] = (int)bufferOneData++;
            fullBuffer.setByte(i, (int)bufferOneData++);
        }
        else
        {
            fullBuffer.setByte(i, (int)bufferTwoData++);
            // fullData[i] = (int)bufferTwoData++;
        }
    }
    ubit->log.beginRow();
    int index;
    // for (int i = 0; i < BUFFER_BUFFER; i++)
    // {
    //     // if (buffers[i]->found)
    //     // {
    //     DMESG("%d:", i);
    //     if (APPROACH == 0)
    //     {
    //         int index = slidingWindow(buffers[i]->buffer);
    //         if (index >= 0)
    //         {

    //             ubit->log.logData("BUF", i);
    //             ubit->log.logData("WINDOW", index);
    //             ubit->log.logData("RAW AUD", (int)buffers[i]->time);
    //             ubit->log.logData("MAG", (int)buffers[i]->mag);
    //             // if (frontIndexFlag)
    //             // {
    //             time = buffers[i]->time + (SAMPLE_LENGTH_US * (WINDOW_SIZE / SPLIT_NUMBER)) *
    //             index; this->timeTakenUS = i * (SAMPLE_SIZE * SAMPLE_LENGTH_US) +
    //                                 ((SAMPLE_LENGTH_US * (WINDOW_SIZE / SPLIT_NUMBER)) *
    //                                 index);
    //             ubit->log.logData("ALT TIME", timeTakenUS);
    //             PRINTFLOATMSG("TIME", time);
    //             // ubit->display.print(time);
    //             // }
    //             // else
    //             // {
    //             //     time = buffers[i]->time + (SAMPLE_LENGTH_MS * (index / SPLIT_NUMBER))
    //             -
    //             //     20; PRINTFLOATMSG("TIME", time);
    //             // }
    //             fiber_sleep(1);
    //             break;
    //         }
    //     }
    //     else
    //     {
    //         this->addSamples(0, WINDOW_SIZE, buffers[i]->buffer);
    //         bool a = this->f->processReal();
    //         this->addSamples(WINDOW_SIZE, SAMPLE_SIZE, buffers[i]->buffer);
    //         bool b = this->f->processReal();
    //         if (a || b)
    //         {
    //             anyFound = true;
    //             index = this->slidingWindowTwo(buffers[i]->buffer);
    //             if (index >= 0)
    //             {
    //                 ubit->log.logData("BUF", i);
    //                 ubit->log.logData("SAMPLE", index);
    //                 ubit->log.logData("RAW AUD", (int)buffers[i]->time);
    //                 ubit->log.logData("MAG", (int)buffers[i]->mag);
    //                 this->time = buffers[i]->time + (SAMPLE_LENGTH_US * index);
    //                 this->timeTakenUS = (((i - 1) * SAMPLE_SIZE) + index) * SAMPLE_LENGTH_US;
    //                 break;
    //             }
    //         }
    //     }
    // }

    index = this->slidingWindowTwo(fullBuffer);
    if (index == -1)
    {
        DMESG("NOT FOUND FREQUENCY");
        fiber_sleep(10);
        return false;
    }
    ubit->log.logData("SAMPLE", index);
    ubit->log.logData("RAW AUD", (int)buffers[1]->time);
    this->time = buffers[1]->time + (SAMPLE_LENGTH_US * index);
    this->timeTakenUS = SAMPLE_LENGTH_US * index;
    ubit->log.logData("ALT TIME", timeTakenUS);
    ubit->display.print("T");
    DMESG("RETURN TRUE");
    fiber_sleep(1);
    return true;
}