#include "MicroBit.h"
#include "DataStream.h"
#include "global.h"

#ifndef MIC_SAMPLER_H
#define MIC_SAMPLER_H

class MicSampler : public DataSink
{
public:
    MicSampler(DataSource &s, MicroBit *ubit);
    ~MicSampler();
    virtual int pullRequest();

    void start() { this->active = true; }
    void stop() { this->active = false; }
    ManagedBuffer getBuffer()
    {
        time = ubit->systemTime();
        return this->buffer;
    }
    long getTime() { return this->time; }

private:
    MicroBit *ubit;
    DataSource &source;
    bool active = false;
    long time = 0;
    ManagedBuffer buffer;
};

#endif