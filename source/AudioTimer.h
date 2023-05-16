#include "Timer.h"
#ifndef AUDIOTIMER_H
#define AUDIOTIMER_H

namespace AudioTimer
{

extern long audioTime;
extern codal::Timer *audioTimer;

extern void setTimer(codal::Timer *t);
} // namespace AudioTimer

#endif