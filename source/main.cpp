#include "MicroBit.h"
#include "samples/Tests.h"
#include "MicSampler.h"
#include "global.h"

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
        playTone(TRANSMIT_FREQUENCY, 20, 2000);
    }
}

void distanceCalculation(long audioTime, clock_t aTime2)
{
    uBit.display.print("C");
    double time = audioTime - radioRecvTime;
    PRINTFLOATMSG("UBIT CLOCK", time);
    PRINTFLOATMSG("RAW CLOCK", aTime2);
    PRINTFLOATMSG("SYS CLOCK", (aTime2 - rRecv) * 1000 / 64000000);
    double distance = SPEEDOFSOUND * (time / 1000.0f);
    PRINTFLOATMSG("DISTANCE CALCULATED", distance);

    fiber_sleep(1000);
    // send();
    uBit.reset();
    // uBit.radio.enable();
    // while (1)
    // {
    //     fiber_sleep(1);
    // }
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
    while (true)
    {
        // audioRecvTime = uBit.systemTime();
        // ManagedBuffer buf = sampler->getBuffer();
        // PRINTFLOATMSG("BUFFER SIZE", buf.length());

        // if (sampler->foundResult() && radioPulse)
        // {
        //     fiber_sleep(1);
        //     DMESG("FREQUENCY DETECTED");
        //     fiber_sleep(1);
        //     // PRINTFLOATMSG("TIME DIFFERENCE", sampler->getTime() - radioRecvTime);
        //     fiber_sleep(1);
        //     DMESG("END");
        //     fiber_sleep(1);
        //     break;
        // }
        // else if (sampler->foundResult() && !radioPulse)
        // {
        //     DMESG("NO RADIO PULSE");
        //     fiber_sleep(1);
        //     sampler->goAgain();
        // }
        DMESG("LOOP");
        if (sampler->foundResult())
        {
            DMESG("SAMPLER HAS FINISHED");
            sampler->processResult();
        }
        fiber_sleep(10);
        // if (uBit.systemTime() - time > 5000)
        // {
        //     sampler->stop();
        //     DMESG("TIMEOUT");
        //     fiber_sleep(1);
        //     timedOut = true;
        //     break;
        // }
        // DMESG("TIME: %d", (int)(uBit.systemTime() - time));
        // PRINTFLOATMSG("TIME TO LOOP", uBit.systemTime() - audioRecvTime);
    }
    // fiber_sleep(20);
    PRINTFLOATMSG("GOING TO SEND", uBit.systemTime());
    fiber_sleep(1);
    uBit.display.setBrightness(255);
    // playTone(363, 1000, 10);
    radioPulse = false;
    if (!timedOut)
        distanceCalculation(sampler->getTime(), sampler->aRecv);
    else
    {
        uBit.reset();

        send();
    }
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
