#include "MicroBit.h"

#define RAWFLOAT(f) (int)f, abs((int)(f * 1000.0f - ((int)f * 1000.0)))
#define PRINTFLOAT(f) DMESG("%d.%d", RAWFLOAT(f))
#define PRINTTWOFLOATMSG(msg, f1, f2) DMESG("%s: %d.%d|%d.%d", msg, RAWFLOAT(f1), RAWFLOAT(f2))
#define PRINTFLOATMSG(msg, f) DMESG("%s: %d.%d", msg, RAWFLOAT(f))
#define PRINTFOURFLOAT(f1, f2, f3, f4)                                                             \
    DMESG("%d.%d\t%d.%d\t%d.%d\t%d.%d", RAWFLOAT(f1), RAWFLOAT(f2), RAWFLOAT(f3), RAWFLOAT(f4))
#define PRINTFIVEFLOAT(f1, f2, f3, f4, f5)                                                         \
    DMESG("%d.%d\t|%d.%d\t|%d.%d\t|%d.%d\t|%d.%d", RAWFLOAT(f1), RAWFLOAT(f2), RAWFLOAT(f3),       \
          RAWFLOAT(f4), RAWFLOAT(f5))

#define PRINTCOMPLEX(r, i) DMESG("%d.%d + %d.%di", RAWFLOAT(r), RAWFLOAT(i))

#define PRINTBUFFER(b)                                                                             \
    DMESG("BUFFER %d: M: %d\tF: %s T: %d", b->index, b->mag, (b->found ? "true " : "false"),       \
          b->time)

#define BUFFER_SIZE 256

#define WINDOW_SIZE 128

#define TRANSMIT_FREQUENCY 2700

#define RECVTIMEOUT 5000

#define MIC_SAMPLE_RATE 45454 // apparently

#define SPEEDOFSOUND_MS 343 // m/s

#define SPEEDOFSOUND_MMS 0.343 // m/ms

#define SPEEDOFSOUND_MUS 0.000343 // m/us

#define SPEEDOFSOUND_CMUS 0.0343 // cm/us

#define BUFFER_BUFFER 5

#define SPLIT_NUMBER 8

#define BUFFER_LENGTH_MS 23

#define SAMPLE_LENGTH_US 90

#define BUFFER_LENGTH_US SAMPLE_LENGTH_US * BUFFER_SIZE

#define APPROACH 1

#define SLIDINGWINDOWSIZE 16

#define CHIRPLENGTH_MS 20

#define CHIRPLENGTH_US 50000

#define MAG_THRESHOLD 15
