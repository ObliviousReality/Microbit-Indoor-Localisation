#include "MicroBit.h"

#define RAWFLOAT(f) (int)f, abs((int)(f * 1000.0f - ((int)f * 1000.0)))
#define PRINTFLOAT(f) DMESG("%d.%d", RAWFLOAT(f));
#define PRINTFLOATMSG(msg, f) DMESG("%s: %d.%d", msg, RAWFLOAT(f));
#define PRINTFOURFLOAT(f1, f2, f3, f4) DMESG("%d.%d %d.%d %d.%d %d.%d", RAWFLOAT(f1), RAWFLOAT(f2), RAWFLOAT(f3), RAWFLOAT(f4))
#define PRINTFIVEFLOAT(f1, f2, f3, f4, f5) DMESG("%d.%d %d.%d %d.%d %d.%d %d.%d", RAWFLOAT(f1), RAWFLOAT(f2), RAWFLOAT(f3), RAWFLOAT(f4), RAWFLOAT(f5))