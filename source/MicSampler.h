#include "MicroBit.h"
#include "DataStream.h"
#include "global.h"

#ifndef MIC_SAMPLER_H
#define MIC_SAMPLER_H

class MicSampler : public DataSink
{
public:
    MicSampler(DataSource &s);
    ~MicSampler();
    virtual int pullRequest();

    void start() { this->active = true; }
    void stop() { this->active = false; }
    ManagedBuffer getBuffer() { return this->buffer; }

private:
    DataSource &source;
    bool active = false;
    ManagedBuffer buffer;
};

#endif