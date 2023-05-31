#include "MicSampler.h"
#include <time.h>
#include "AudioTimer.h"

MicSampler::MicSampler(DataSource &s, MicroBit *ubit) : source(s)
{
    source.connect(*this);
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
    if (!ubit->audio.mic->isEnabled())
    {
        DMESG("MIC DISABLED, RESETTING");
        ubit->reset();
    }
    long localtime = AudioTimer::audioTime - BUFFER_LENGTH_US;
    buffer = source.pull();
    if (terminating == 1)
    {
        DMESG("TERMINATING");
        TheBufferTime = localtime;
        TheBuffer = buffer;
        // this->oneMore();
        this->stop();
    }

    return DEVICE_OK;
}

bool MicSampler::processResult(long radioTime)
{
    bool firstFound = false;
    this->addSamples(0, WINDOW_SIZE, TheBuffer);
    bool a = this->f->processReal();
    PRINTFLOATMSG("FFT1 MAG2.7", this->f->getMag());
    PRINTFLOATMSG("FFT1 MAG5.4", this->f->getMagTwo());
    this->addSamples(WINDOW_SIZE, SAMPLE_SIZE, TheBuffer);
    bool b = this->f->processReal();
    PRINTFLOATMSG("FFT2 MAG2.7", this->f->getMag());
    PRINTFLOATMSG("FFT2 MAG5.4", this->f->getMagTwo());
    if (a || b)
    {
        firstFound = true;
    }
    ManagedBuffer bufferGoingForwards;
    if (firstFound)
    {
        bufferGoingForwards = TheBuffer;
    }
    else
    {
        DMESG("NOT FOUND");
        fiber_sleep(1);
        // ubit->reset();
    }

    // ManagedBuffer fullBuffer = ManagedBuffer((SAMPLE_SIZE) + (2 * SLIDINGWINDOWSIZE));
    // int8_t *bufferData = (int8_t *)&TheBuffer[0];
    // for (int i = 0; i < SLIDINGWINDOWSIZE - 1; i++)
    // {
    //     fullBuffer.setByte(i, 0);
    // }
    // for (int i = SLIDINGWINDOWSIZE; i < TheBuffer.length() + SLIDINGWINDOWSIZE; i++)
    // {
    //     // PRINTFLOAT(TheBuffer[i]);
    //     // DMESG("%d", (int)bufferData);
    //     int item = (int)*bufferData++;
    //     DMESG("%d ", item);
    //     if (item == 1 || item == 255 || item == 254)
    //     {
    //         item = 0;
    //     }

    //     fullBuffer.setByte(i, item);
    // }
    // for (int i = TheBuffer.length() + SLIDINGWINDOWSIZE;
    //      i < (SAMPLE_SIZE) + (2 * SLIDINGWINDOWSIZE); i++)
    // {
    //     fullBuffer.setByte(i, 0);
    // }
    int index = this->slidingWindow(TheBuffer);
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