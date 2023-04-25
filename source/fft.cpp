#include "MicroBit.h"
#include "fft.h"


FFT::FFT(DataSource &source) : audioStream(source)
{
    audioStream.connect(*this);
}

int FFT::pullRequest() {
    auto samples = audioStream.pull();
    int16_t *data = (int16_t *) &samples[0];
    this->dataPoint = data[0];
    return DEVICE_OK;
}
