#include "MicroBit.h"
#include "DataStream.h"
#include "global.h"
#include "fft.h"

#ifndef MIC_SAMPLER_H
#define MIC_SAMPLER_H

struct AudioBuffer
{
    AudioBuffer(int i, ManagedBuffer b, long t)
    {
        this->index = i;
        this->buffer = b;
        this->time = t;
    }
    int index;
    ManagedBuffer buffer;
    long time;
    int mag = 0;
    bool found = false;
    // int *subMags = (int *)malloc(sizeof(int) * SPLIT_NUMBER);
    // bool *subFounds = (bool *)malloc(sizeof(bool) * SPLIT_NUMBER);
};

class MicSampler : public DataSink
{
public:
    MicSampler(DataSource &s, MicroBit *ubit);
    ~MicSampler();
    virtual int pullRequest();

    bool processResult();

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

    int timeTakenUS = 0;

    bool foundResult() { return this->outcome; }
    clock_t aRecv = 0;

private:
    void addSamples(int start, int end, ManagedBuffer b);
    void binaryChop();
    int slidingWindow(ManagedBuffer b);

    bool processFFT();

    MicroBit *ubit;
    DataSource &source;
    bool active = false;
    double time = 0;

    ManagedBuffer buffer;
    FFT *f = new FFT();
    bool outcome = false;

    int lowerBound, upperBound;

    int bufCounter = 0;
    AudioBuffer **buffers = (AudioBuffer **)malloc(sizeof(AudioBuffer *) * BUFFER_BUFFER);
};

#endif
