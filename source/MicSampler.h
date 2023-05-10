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

    void goAgain() { this->outcome = false; }
    ManagedBuffer getBuffer() { return this->buffer; }
    long getTime() { return this->time; }

    bool foundResult() { return this->outcome; }
    clock_t aRecv = 0;

private:
    void addSamples(int start, int end);
    void testRanges();

    MicroBit *ubit;
    DataSource &source;
    bool active = false;
    long time = 0;

    ManagedBuffer buffer;
    FFT *f = new FFT();
    bool outcome = false;

    int lowerBound, upperBound;

    // int timeTotal = 0;
    // int timeCounter = 0;
};

#endif
