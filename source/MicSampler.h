#include "MicroBit.h"
#include "DataStream.h"

#ifndef MIC_SAMPLER_H
#define MIC_SAMPLER_H

#define WINDOW_SIZE 128

class MicSampler : public DataSink
{
public:
    MicSampler(DataSource &s);
    ~MicSampler();
    virtual int pullRequest();

    void start()
    {
        this->active = true;
        uBit.audio.activateMic();
    }
    void stop()
    {
        this->active = false;
        uBit.audio.deactivateMic();
    }

    int getLevel() { return this->level; }
    int getMax() { return this->max; }
    int getTotal() { return this->total; }
    ManagedBuffer *getBuffer() { return this->buffer; }

private:
    DataSource &source;
    int windowSize = WINDOW_SIZE; // May be worth changing.

    int windowPos = 0;
    int workTotal = 0;
    int workMax = -10000;

    bool active = false;

    int level = 0;
    int max = 0;
    int total = 0;
    int16_t *buf;
    ManagedBuffer *buffer;
};

#endif