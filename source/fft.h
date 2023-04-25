#include "MicroBit.h"
#include "DataStream.h"
#define ARM_MATH_CM4
#include "arm_math.h"

#ifndef FFT_H
#define FFT_H

#define SAMPLE_RATE (11 * 1024)
#define SAMPLE_NUM 1024

class FFT : public DataSink
{
    DataSource &audioStream;

public:
    FFT(DataSource &s);
    ~FFT();
    virtual int pullRequest();
    // int getFreq();
    // int setDivisor(int d);
    // void start();
    // void stop(MicroBit &ubit);

    int16_t dataPoint = 0;

private:
};

#endif