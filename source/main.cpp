#include "MicroBit.h"
#include "samples/Tests.h"
#include "MicSampler.h"
#include "fft.h"

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

MicSampler *sampler = new MicSampler(*uBit.audio.splitter->createChannel());
FFT *f = new FFT();

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
        playTone(TRANSMIT_FREQUENCY, 20, 1000);
    }
}

void recv()
{
    bool timeoutTriggered = false;
    long time = uBit.systemTime();
    PRINTFLOATMSG("LOOP START", time);
    while (true)
    {
        audioRecvTime = uBit.systemTime();
        ManagedBuffer buf = sampler->getBuffer();
        int16_t *data = (int16_t *)&buf[0];
        f->clearSamples();
        for (int i = 0; i < WINDOW_SIZE; i++)
        {
            f->addSample((int8_t)*data);
            data++;
        }
        bool result = f->processReal();
        if (result && radioRecvTime)
        {
            DMESG("FREQUENCY DETECTED");
            timeoutTriggered = true;
            // uBit.radio.datagram.send("thanks it worked");
            PRINTFLOATMSG("TIME DIFFERENCE", audioRecvTime - radioRecvTime);
        }
        // DMESG("TIME: %d", (int)(uBit.systemTime() - time));
        if (timeoutTriggered)
        {
            DMESG("END");
            sampler->stop();
            break;
        }
        fiber_sleep(1);
    }
    fiber_sleep(20);
    PRINTFLOATMSG("GOING TO SEND", uBit.systemTime());
    uBit.display.setBrightness(255);
    uBit.radio.enable();
    send();
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
    PacketBuffer b = uBit.radio.datagram.recv();
    DMESG("MESSAGE FROM: %s", b.getBytes());
    radioRecvTime = uBit.systemTime();
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

    sampler->start();

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