#include "AudioTimer.h"
#include "Timer.h"

namespace AudioTimer
{
long audioTime = 0;
codal::Timer *audioTimer = nullptr;
void setTimer(codal::Timer *t)
{
    audioTimer = t;
}
} // namespace AudioTimer
