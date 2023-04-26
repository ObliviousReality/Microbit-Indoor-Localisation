#include "MicroBit.h"
#include "DataStream.h"

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

    int getLevel() { return this->level; }
    int getMax() { return this->max; }
    int getTotal() { return this->total; }

private:
    DataSource &source;
    int windowSize = 128;
    int windowPos = 0;
    int workTotal = 0;
    int workMax = -10000;
    bool active = false;
    int level = 0;
    int max = 0;
    int total = 0;
};

#endif