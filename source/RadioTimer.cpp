#include "RadioTimer.h"
#include "Timer.h"

namespace RadioTimer
{
volatile long radioTime = 0;
codal::Timer *radioTimer = nullptr;
volatile bool pulseReceived = false;
} // namespace RadioTimer
