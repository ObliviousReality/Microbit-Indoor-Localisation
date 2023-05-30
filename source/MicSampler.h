#include "MicroBit.h"
#include "DataStream.h"
#include "global.h"
#include "fft.h"

#ifndef MIC_SAMPLER_H
#define MIC_SAMPLER_H

class MicSampler : public DataSink
{
public:
    MicSampler(DataSource &s, MicroBit *ubit);
    ~MicSampler();
    virtual int pullRequest();

    bool processResult(long radioTime);

    void start()
    {
        this->active = true;
        this->outcome = false;
    }
    void stop()
    {
        this->active = false;
        this->outcome = true;
    }

    void terminate() { this->terminating = 1; }

    ManagedBuffer getBuffer() { return this->buffer; }
    long getTime() { return this->time; }

    bool foundResult() { return this->outcome; }

private:
    void addSamples(int start, int end, ManagedBuffer b);
    int slidingWindow(ManagedBuffer b);

    bool processFFT();

    MicroBit *ubit;
    DataSource &source;
    bool active = false;
    double time = 0;

    ManagedBuffer buffer;
    FFT *f = new FFT();
    bool outcome = false;

    ManagedBuffer TheBuffer;
    long TheBufferTime;
    int terminating = 0;
};

#endif
