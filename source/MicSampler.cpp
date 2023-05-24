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
    int curMag = 0;
    int prevMag = 0;
    bool curFound = false;
    bool prevFound = false;
    for (int i = 0; i < b.length() - SLIDINGWINDOWSIZE; i++)
    {
        this->addSamples(i, i + SLIDINGWINDOWSIZE, b);
        curFound = this->f->processReal();
        curMag = this->f->getMag();
        if (i != 0)
        {
            if ((curFound && !prevFound))
            {
                return i;
            }
        }
        prevFound = curFound;
        prevMag = curMag;
    }
    return -1;
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
                               BUFFER_LENGTH_US); // roughly every 23 ms but does slightly vary.
    time = AudioTimer::audioTime - BUFFER_LENGTH_US;
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
    ManagedBuffer fullBuffer = ManagedBuffer((SAMPLE_SIZE * 2) + (2 * SLIDINGWINDOWSIZE));
    int tempBufInd = 1;
    int16_t *bufferData = (int16_t *)&buffers[1]->buffer[0];
    for (int i = 0; i < (SAMPLE_SIZE * 4) + (2 * SLIDINGWINDOWSIZE); i++)
    {
        if (i < SLIDINGWINDOWSIZE || i > (SAMPLE_SIZE * 4) + SLIDINGWINDOWSIZE)
        {
            fullBuffer.setByte(i, 0);
        }
        if (i == SLIDINGWINDOWSIZE + ((tempBufInd + 1) * SAMPLE_SIZE))
        {
            bufferData = (int16_t *)&buffers[++tempBufInd]->buffer[0];
        }
        if (!tempBufInd)
        {
            fullBuffer.setByte(i, (int)bufferData++);
        }
        else
        {
            fullBuffer.setByte(i, (int)bufferData++);
        }
    }

    ubit->log.beginRow();
    int index = this->slidingWindow(fullBuffer) - 256;

    if (index == -1)
    {
        DMESG("NOT FOUND FREQUENCY");
        fiber_sleep(10);
        return false;
    }

    // int estimatedEndSample = index + (CHIRPLENGTH_US / SAMPLE_LENGTH_US);

    // this->addSamples(estimatedEndSample - 16, estimatedEndSample, fullBuffer);
    // bool lastWindow = this->f->processReal();
    // int lastWindowMag = this->f->getMag();

    // this->addSamples(estimatedEndSample + 1, estimatedEndSample + 17, fullBuffer);
    // bool nextWindow = this->f->processReal();
    // int nextWindowMag = this->f->getMag();

    ubit->log.logData("SAMPLE", index);
    ubit->log.logData("RAW AUD", (int)buffers[1]->time);
    this->time = buffers[1]->time + (SAMPLE_LENGTH_US * index);
    this->timeTakenUS = SAMPLE_LENGTH_US * index;
    ubit->log.logData("ALT TIME", timeTakenUS);
    // ubit->log.logData("lastWindow", lastWindow);
    // ubit->log.logData("nextWindow", nextWindow);
    ubit->display.print("T");
    DMESG("RETURN TRUE");
    fiber_sleep(1);
    return true;
}