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
    int N = 2;
    int found = 0;
    int arr[b.length()];
    int freqs[b.length()];
    int ctr = 0;
    for (int i = startPoint; i < b.length() - SLIDINGWINDOWSIZE; i++)
    {
        this->addSamples(i, i + SLIDINGWINDOWSIZE, b);
        arr[ctr] = this->f->processReal() ? 1 : 0;
        freqs[ctr++] = this->f->frequency;
        // bool res = this->f->processReal();
        // if ()
        // {
        //     found++;
        //     if (found == N)
        //         return i - N;
        //     // PRINTTWOFLOATMSG("T", this->f->getMag(), this->f->frequency);
        // }
        // else
        // {
        //     found = 0;
        // }
        // PRINTTWOFLOATMSG("F", this->f->getMag(), this->f->frequency);
    }
    DMESG("__");
    int returnVal = -1;
    bool set = false;
    for (int i = 0; i < ctr; i++)
    {
        DMESG("%d,%d", arr[i], freqs[i]);
        if (arr[i] == 1 && freqs[i] == 3470 && !set)
        {
            found = 0;
            for (int j = 0; j < N; j++)
            {
                if (arr[i + j] == 1 && freqs[i + j] == 3470)
                {
                    found++;
                }
                if (found >= N)
                {
                    returnVal = startPoint + i;
                    set = true;
                }
            }
        }
    }
    DMESG("--");
    return returnVal;
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
    // DMESG("BUFFER TIME: %d", AudioTimer::audioTime);
    if (doAnother)
    {
        DMESG("DOING ANOTHER");
        ManagedBuffer newBuffer = ManagedBuffer((BUFFER_SIZE * 2));
        for (int i = 0; i < BUFFER_SIZE; i++)
        {
            newBuffer.setByte(i, TheBuffer[i]);
        }
        for (int i = BUFFER_SIZE; i < (BUFFER_SIZE * 2); i++)
        {
            newBuffer.setByte(i, buffer[i - BUFFER_SIZE]);
        }
        TheBuffer = newBuffer;
        this->stop();
    }
    if (terminating == 1)
    {
        DMESG("TERMINATING");
        fiber_sleep(1);
        if (bufCounter == 8)
            TheBufferTime = localtime;
        // TheBuffer = buffer;
        int ctr = 0;
        for (int i = 256 * (8 - bufCounter); i < buffer.length(); i++)
        {
            TheBuffer[i] = buffer[ctr++];
        }
        bufCounter--;
        // this->oneMore();
        if (bufCounter == 0)
        {
            bufCounter = 8;
            this->stop();
        }
        return DEVICE_OK;
    }
    return DEVICE_OK;
}

bool MicSampler::processResult(long radioTime)
{
    bool firstFound = false;
    this->addSamples(0, 1024, TheBuffer);
    bool a = this->f->processReal();
    DMESG("FREQ1: %d", f->frequency);
    // PRINTFLOATMSG("FFT1 MAG2.7", this->f->getMag());
    // PRINTFLOATMSG("FFT1 MAG5.4", this->f->getMagTwo());
    this->addSamples(1024, TheBuffer.length(), TheBuffer);
    bool b = this->f->processReal();
    DMESG("FREQ2: %d", f->frequency);
    // PRINTFLOATMSG("FFT2 MAG2.7", this->f->getMag());
    // PRINTFLOATMSG("FFT2 MAG5.4", this->f->getMagTwo());
    if (a || b)
    {
        firstFound = true;
    }
    else
    {
        // DMESG("NOT FOUND");
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

    int index = this->slidingWindow(TheBuffer, radioSample);
    // DMESG("--");
    // if ((index == radioSample) || (index == radioSample + 1))
    // {
    //     DMESG("TOO CLOSE");
    //     fiber_sleep(1);
    //     return false;
    // }
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
    // DMESG("RADIOSAMPLE: %d", radioSample);
    if (radioSample > index && index >= 0)
    {
        DMESGF("CHIRP BEFORE RADIO");
        DMESG("%d,%d", radioSample, index);
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
    // DMESG("INDEX: %d", index);
    this->time = TheBufferTime + (SAMPLE_LENGTH_US * index);
    return true;
}