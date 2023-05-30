#include "MicSampler.h"
#include <time.h>
#include "AudioTimer.h"

MicSampler::MicSampler(DataSource &s, MicroBit *ubit) : source(s)
{
    source.connect(*this);
    // DMESG("SAMPLER ONLINE");
    // PRINTFLOATMSG("SAMPLE RATE OF SAMPLER", source.getSampleRate());
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
    int8_t *data = (int8_t *)&b[0];
    data = data + start;
    // DMESG("ADDING SAMPLE 1: %d", (int8_t)*data);
    int i;
    for (i = start; i < end; i++)
    {
        int item = (int8_t)*data++;
        if (item == 1 || item == 255 || item == 254)
        {
            item = 0;
        }
        this->f->addSample(item);
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
    for (int i = SLIDINGWINDOWSIZE; i < b.length() - SLIDINGWINDOWSIZE; i++)
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
    // if (bufCounter == BUFFER_BUFFER)
    // {
    //     DMESG("STOPPING, BUFFER FILLED");
    //     this->stop();
    //     fiber_sleep(10);
    //     return DEVICE_OK;
    // }
    if (!ubit->audio.mic->isEnabled())
    {
        DMESG("MIC DISABLED, RESETTING");
        ubit->reset();
    }
    // PRINTFLOATMSG("PR AT", AudioTimer::audioTime -
    //                            BUFFER_LENGTH_US); // roughly every 23 ms but does slightly vary.
    long localtime = AudioTimer::audioTime - BUFFER_LENGTH_US;
    buffer = source.pull();
    aRecv = clock();
    // DMESG("BUF COUNTER: %d", this->bufCounter);
    if (this->doingAnother)
    {
        TheBufferTwo = buffer;
        TheBufferTimeTwo = localtime;
        this->stop();
    }
    else
    {
        // buffers[this->bufCounter] = new AudioBuffer(buffer, localtime);
        // this->bufCounter++;
        // if (bufCounter >= BUFFER_BUFFER)
        // {
        //     bufCounter = 0;
        // }
        if (terminating == 1)
        {
            DMESG("TERMINATING");
            TheBufferTime = localtime;
            TheBuffer = buffer;
            // this->oneMore();
            this->stop();
        }
    }
    return DEVICE_OK;
}

bool MicSampler::processResult(long radioTime)
{
    DMESG("PROCESS TIME");
    bool anyFound = false;
    int sampleStartBuffer = 100;
    for (int i = 0; i < BUFFER_BUFFER; i++)
    {
        if (buffers[i]->time > radioTime)
        {
            if (sampleStartBuffer > i)
                sampleStartBuffer = i;
            this->addSamples(0, WINDOW_SIZE, buffers[i]->buffer);
            bool a = this->f->processReal();
            this->addSamples(WINDOW_SIZE, SAMPLE_SIZE, buffers[i]->buffer);
            bool b = this->f->processReal();
            if (a || b)
            {
                anyFound = true;
            }
        }
    }

    if (!anyFound)
    {
        DMESG("NOT FOUND FREQUENCY");
        fiber_sleep(10);
        return false;
    }
    ManagedBuffer fullBuffer = ManagedBuffer((SAMPLE_SIZE * 4) + (2 * SLIDINGWINDOWSIZE));
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

    ubit->log.logData("BUFFER", (index) / 256);
    ubit->log.logData("SSB", sampleStartBuffer);
    ubit->log.logData("BC", bufCounter);
    ubit->log.logData("SAMPLE", index);
    ubit->log.logData("RAW AUD", (int)buffers[1]->time);
    this->time = buffers[1]->time + (SAMPLE_LENGTH_US * index);
    ubit->display.print("T");
    DMESG("RETURN TRUE");
    fiber_sleep(1);
    return true;
}

bool MicSampler::processResult2(long radioTime)
{
    bool firstFound = false;
    this->addSamples(0, WINDOW_SIZE, TheBuffer);
    bool a = this->f->processReal();
    this->addSamples(WINDOW_SIZE, SAMPLE_SIZE, TheBuffer);
    bool b = this->f->processReal();
    if (a || b)
    {
        firstFound = true;
    }
    // bool secondFound = false;
    // this->addSamples(0, WINDOW_SIZE, TheBufferTwo);
    // a = this->f->processReal();
    // this->addSamples(WINDOW_SIZE, SAMPLE_SIZE, TheBufferTwo);
    // b = this->f->processReal();
    // if (a || b)
    // {
    //     secondFound = true;
    // }
    ManagedBuffer bufferGoingForwards;
    if (firstFound)
    {
        bufferGoingForwards = TheBuffer;
    }
    // else if (secondFound)
    // {
    //     bufferGoingForwards = TheBufferTwo;
    // }
    else
    {
        DMESG("NOT FOUND");
        fiber_sleep(1);
        // ubit->reset();
    }

    ManagedBuffer fullBuffer = ManagedBuffer((SAMPLE_SIZE) + (2 * SLIDINGWINDOWSIZE));
    int8_t *bufferData = (int8_t *)&TheBuffer[0];
    for (int i = 0; i < SLIDINGWINDOWSIZE - 1; i++)
    {
        fullBuffer.setByte(i, 0);
    }
    for (int i = SLIDINGWINDOWSIZE; i < TheBuffer.length() + SLIDINGWINDOWSIZE; i++)
    {
        // PRINTFLOAT(TheBuffer[i]);
        // DMESG("%d", (int)bufferData);
        int item = (int)*bufferData++;
        DMESG("%d ", item);
        if (item == 1 || item == 255 || item == 254)
        {
            item = 0;
        }

        fullBuffer.setByte(i, item);
    }
    for (int i = TheBuffer.length() + SLIDINGWINDOWSIZE;
         i < (SAMPLE_SIZE) + (2 * SLIDINGWINDOWSIZE); i++)
    {
        fullBuffer.setByte(i, 0);
    }
    int index = this->slidingWindow(fullBuffer) - SLIDINGWINDOWSIZE;
    if (index < 0)
    {
        DMESG("NEGATIVE INDEX");
        fiber_sleep(1);
        ubit->reset();
    }
    int radioSample = (radioTime - TheBufferTime) / 90;
    if (radioSample > index)
    {
        DMESG("CHIRP BEFORE RADIO");
        fiber_sleep(1);
        ubit->reset();
    }
    ubit->log.beginRow();

    ubit->log.logData("FOUND", firstFound ? "True" : "False");
    // ubit->log.logData("FOUND2", secondFound ? "True" : "False");
    ubit->log.logData("SAMPLE", index);
    ubit->log.logData("RADIO SAMPLE", radioSample);
    ubit->log.logData("RAW AUD", (int)TheBufferTime);
    ubit->log.logData("SDIST",
                      (int)(((index - radioSample) * SAMPLE_LENGTH_US) * SPEEDOFSOUND_CMUS));
    ubit->log.logData("SAMPLE DIFF", index - radioSample);
    DMESG("INDEX: %d", index);
    this->time = TheBufferTime + (SAMPLE_LENGTH_US * index);
    return true;
}