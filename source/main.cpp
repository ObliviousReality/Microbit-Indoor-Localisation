#include "MicroBit.h"
#include "samples/Tests.h"
#include "MicSampler.h"
#include "global.h"

#include "AudioTimer.h"
#include "RadioTimer.h"

#include <time.h>
// #include <sys/time.h>

static void radioReceive(MicroBitEvent);
void recv();

MicroBit uBit;
SoundOutputPin *pin = &uBit.audio.virtualOutputPin;

char prompts[] = {'<', 'S', '<', ' ', '>', 'R', '>', ' '};
int promptCounter = 0;
const int MAXPROMPTS = 8;

char name[7];

long radioRecvTime = 0;
long audioRecvTime = 0;

clock_t rRecv;

bool radioPulse = false;

void playTone(int f, int hiT, int loT = -1)
{
    if (loT < 0)
        loT = hiT;
    pin->setAnalogValue(128);
    pin->setAnalogPeriodUs(1000000 / f);
    fiber_sleep(hiT);
    pin->setAnalogValue(0);
    fiber_sleep(loT);
}

void send()
{
    DMESG("SEND");
    uBit.display.print("S");
    while (true)
    {
        uBit.radio.datagram.send(name);
        // fiber_sleep(20);
        playTone(TRANSMIT_FREQUENCY, 50, 1000);
        // uBit.sleep(500);
        // uBit.reset();
    }
}

void distanceCalculation(long samplerTime)
{
    uBit.display.print("C");
    long timeDiff_US = samplerTime - RadioTimer::radioTime;
    uBit.log.logData("RAD TIME (us)", (int)RadioTimer::radioTime);
    uBit.log.logData("AUD TIME (us)", (int)samplerTime);
    uBit.log.logData("DIFF (us)", (int)timeDiff_US);
    // speed = distance / time
    // distance = speed * time

    double distance = SPEEDOFSOUND_CMUS * timeDiff_US;
    // uBit.display.print(distance);
    PRINTFLOATMSG("TIME DIFFERENCE", timeDiff_US);
    PRINTFLOATMSG("DISTANCE", distance);
    uBit.log.logData("DISTANCE (cm?)", (int)(distance * 1000.0f));
    uBit.log.endRow();
    uBit.display.print("Y");
    uBit.sleep(300);
    uBit.reset();
}

void recv()
{
    MicSampler *sampler = new MicSampler(*uBit.audio.splitter->createChannel(), &uBit);
    uBit.display.print("R");
    long time = uBit.systemTime();
    PRINTFLOATMSG("LOOP START", time);
    radioRecvTime = uBit.systemTime();
    sampler->start();
    bool timedOut = false;
    bool processedAlready = false;

    while (true)
    {
        // DMESG("LOOP");
        if (sampler->foundResult() && !processedAlready)
        {
            processedAlready = true;
            DMESG("SAMPLER HAS FINISHED");
            // uBit.sleep(1000);
            DMESG("-");
            // uBit.sleep(1000);
            bool outcome = sampler->processResult();
            if (outcome)
            {
                DMESG("SUCCESS");
                uBit.display.print("Y");
                break;
            }
            else
            {
                DMESG("FAIL");
                uBit.display.print("N");
                fiber_sleep(300);
                uBit.reset();
            }
        }
        fiber_sleep(1);
    }
    distanceCalculation(sampler->getTime());
}

void test()
{
    DMESG("TEST");
    while (true)
    {
        fiber_sleep(200);
    }
}

static void radioReceive(MicroBitEvent)
{
    radioRecvTime = uBit.systemTime();
    rRecv = clock();
    if (radioPulse)
    {
        return;
    }
    radioPulse = true;
    PacketBuffer b = uBit.radio.datagram.recv();
    DMESG("MESSAGE FROM: %s", b.getBytes());
    PRINTFLOATMSG("MESSAGE RECVD AT", RadioTimer::radioTime);
    recv();
}

void bee()
{
    while (true)
    {
        playTone(8000, 1000, 0);
        // buzz
    }
}

int main()
{
    // struct timeval t;
    // gettimeofday(&t, NULL);
    // PRINTFLOAT(t.tv_sec);
    // PRINTFLOAT(t.tv_usec);
    uBit.init();
    uBit.log.setTimeStamp(TimeStampFormat::Milliseconds);
    // syncTimer = &uBit.timer;
    AudioTimer::setTimer(&uBit.timer);
    RadioTimer::radioTimer = &uBit.timer;

    uBit.sleep(123);
    uint64_t t = uBit.timer.getTimeUs();
    PRINTFLOATMSG("TIME uS", t);
    uint64_t val = uBit.getSerialNumber();
    for (int i = 0; i < 6; i++)
    {
        name[i] = 65 + ((val / 2) % 26);
        val = (int)(val / 2);
    }
    name[6] = '\0';
    DMESG("Hello, I'm %s", name);
    uBit.radio.setGroup(5);
    uBit.messageBus.listen(DEVICE_ID_RADIO, MICROBIT_RADIO_EVT_DATAGRAM, radioReceive);
    uBit.radio.enable();
    PRINTFLOATMSG("SAMPLE RATE", uBit.audio.mic->getSampleRate());

    PRINTFLOATMSG("RECV START", uBit.systemTime());

    // sampler->start();

    // if (uBit.audio.mic->isEnabled())
    // {
    //     DMESG("MIC ENABLED");
    // }
    // else
    // {
    //     DMESG("MIC NOT ENABLED");
    //     fiber_sleep(10);
    //     uBit.reset();
    // }
    // uBit.audio.mic->enable();
    while (true)
    {
        // fiber_sleep(20);
        // uBit.display.print(prompts[promptCounter++]);
        // if (promptCounter > MAXPROMPTS)
        // {
        //     promptCounter = 0;
        // }
        if (uBit.buttonA.isPressed())
        {
            send();
            break;
        }
        else if (uBit.buttonB.isPressed())
        {
            recv();
            break;
        }
        uBit.sleep(500);
    }
}
