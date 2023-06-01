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
    int8_t *data = (int8_t *)&b[start];
    // data = data + start;
    int i;
    for (i = start; i < end; i++)
    {
        this->f->addSample(abs(*data++));
    }
}

int MicSampler::slidingWindow(ManagedBuffer b, int startPoint = 0)
{
    for (int i = startPoint; i < b.length() - SLIDINGWINDOWSIZE; i++)
    {
        this->addSamples(i, i + SLIDINGWINDOWSIZE, b);
        if (this->f->processReal())
        {
            // PRINTTWOFLOATMSG("T", this->f->getMag(), this->f->frequency);
            return i;
        }
        // PRINTTWOFLOATMSG("F", this->f->getMag(), this->f->frequency);
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
    if (doAnother)
    {
        // DMESG("DOING ANOTHER");
        ManagedBuffer newBuffer = ManagedBuffer(BUFFER_SIZE * 2);
        for (int i = 0; i < BUFFER_SIZE; i++)
        {
            newBuffer.setByte(i, TheBuffer[i]);
        }
        for (int i = BUFFER_SIZE; i < BUFFER_SIZE * 2; i++)
        {
            newBuffer.setByte(i, buffer[i - BUFFER_SIZE]);
        }
        TheBuffer = newBuffer;
        this->stop();
    }
    if (terminating == 1)
    {
        // DMESG("TERMINATING");
        TheBufferTime = localtime;
        TheBuffer = buffer;
        this->oneMore();
        // this->stop();
        return DEVICE_OK;
    }
    prevMag = this->f->getMag();
    return DEVICE_OK;
}

bool MicSampler::processResult(long radioTime)
{
    // DMESG("PREV MAG %d", prevMag);
    ubit->sleep(1);
    bool firstFound = false;
    this->addSamples(0, WINDOW_SIZE, TheBuffer);
    bool a = this->f->processReal();
    // PRINTFLOATMSG("FFT1 MAG2.7", this->f->getMag());
    // PRINTFLOATMSG("FFT1 MAG5.4", this->f->getMagTwo());
    this->addSamples(WINDOW_SIZE, BUFFER_SIZE, TheBuffer);
    bool b = this->f->processReal();
    // PRINTFLOATMSG("FFT2 MAG2.7", this->f->getMag());
    // PRINTFLOATMSG("FFT2 MAG5.4", this->f->getMagTwo());
    if (a || b)
    {
        firstFound = true;
    }
    else
    {
        DMESG("NOT FOUND");
        fiber_sleep(1);
        // ubit->reset();
    }
    // DMESG("FOUND CHECK");
    // fiber_sleep(1);
    int radioSample = (radioTime - TheBufferTime) / 90;

    if (radioSample > TheBuffer.length() || radioSample < 0)
    {
        DMESG("RADIO SAMPLE WRONG: %d", radioSample);
        fiber_sleep(1);
        return false;
    }
    int altSample = -1;
    int8_t *bufferData = (int8_t *)&TheBuffer[radioSample + 1];
    // bufferData = bufferData + radioSample;
    for (int i = radioSample + 1; i < TheBuffer.length(); i++)
    {
        int item = (int)*bufferData++;
        // DMESG("%d", item);
        if (abs(item) > 1)
        {
            altSample = i;
            break;
        }
    }

    int index = this->slidingWindow(TheBuffer, radioSample + 1);
    // DMESG("--");
    bufferData = (int8_t *)&TheBuffer[0];
    for (int i = 0; i < TheBuffer.length(); i++)
    {
        int item = (int)*bufferData++;
        int val = 0;
        if (index == i)
        {
            val = -3;
        }
        if (altSample == i)
        {
            val = 4;
        }
        if (radioSample == i)
        {
            val = 5;
        }
        // DMESG("%d,%d", TheBuffer[i] - 128, val);
    }
    // DMESG("--");
    DMESG("RADIOSAMPLE: %d", radioSample);
    if (radioSample > index && index >= 0)
    {
        DMESGF("CHIRP BEFORE RADIO");
        fiber_sleep(1);
        index = this->slidingWindow(TheBuffer, index + 1);
        // ubit->reset();
        return false;
    }
    if (index < 0)
    {
        DMESGF("NEGATIVE INDEX: %d", index);
        fiber_sleep(1);
        // ubit->reset();
        return false;
    }
    ubit->log.beginRow();

    ubit->log.logData("FOUND", firstFound ? "True" : "False");
    ubit->log.logData("SAMPLE", index);
    ubit->log.logData("RADIO SAMPLE", radioSample);
    ubit->log.logData("RAW AUD", (int)TheBufferTime);
    ubit->log.logData("SDIST",
                      (int)(((index - radioSample) * SAMPLE_LENGTH_US) * SPEEDOFSOUND_CMUS));
    ubit->log.logData("SAMPLE DIFF", index - radioSample);
    ubit->log.logData("ALT SAMPLE", altSample - radioSample);
    DMESG("INDEX: %d", index);
    this->time = TheBufferTime + (SAMPLE_LENGTH_US * index);
    return true;
}