#include "Timer.h"

#ifndef RADIOTIMER_H
#define RADIOTIMER_H

namespace RadioTimer
{

extern volatile long radioTime;
extern codal::Timer *radioTimer;
extern volatile bool pulseReceived;
} // namespace RadioTimer

#endif