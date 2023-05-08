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
    ManagedBuffer getBuffer() { return this->buffer; }
    long getTime() { return this->time; }

    bool foundResult() { return this->outcome; }

private:
    MicroBit *ubit;
    DataSource &source;
    bool active = false;
    long time = 0;
    ManagedBuffer buffer;
    FFT *f = new FFT();
    bool outcome = false;
};

#endif
