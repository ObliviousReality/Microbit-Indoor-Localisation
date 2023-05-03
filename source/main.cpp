#include "MicroBit.h"
#include "samples/Tests.h"
#include "MicSampler.h"
#include "fft.h"
#include <cstring>

#define RECVTIMEOUT 1000

static void radioReceive(MicroBitEvent);
void recv();

MicroBit uBit;
SoundOutputPin *pin = &uBit.audio.virtualOutputPin;

char prompts[] = {'<', 'S', '<', ' ', '>', 'R', '>', ' '};
int promptCounter = 0;
const int MAXPROMPTS = 8;

char name[7];

FFT *f;

void initialise()
{

    f = new FFT();
}

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
        playTone(8000, 20, 1000);
    }
}

void recv()
{
    DMESG("RECV");
    uBit.display.setBrightness(0);
    MicSampler *sampler = new MicSampler(*uBit.audio.splitter->createChannel());
    for (int y = 0; y < 5; y++)
    {
        for (int x = 0; x < 5; x++)
        {
            uBit.display.image.setPixelValue(x, y, 255);
        }
    }

    sampler->start();
    // f->clearSamples();

    // double data[] = {-69, -65, -72, -67, -65};
    // for (int i = 0; i < 5; i++)
    // {
    //     f->addSample(data[i]);
    // }

    long time = uBit.systemTime();
    while (true)
    {
        fiber_sleep(20);
        int lev = sampler->getMax();
        if (lev > 1)
        {
            uBit.display.setBrightness(lev);
            // DMESG("CUR LEVEL: %d", lev);
        }
        else
        {
            uBit.display.setBrightness(0);
        }
        // ManagedBuffer *buf = sampler->getBuffer();
        // int16_t *data = (int16_t *)&buf[0];
        // for (int i = 0; i < buf->length(); i++)
        // {
        //     f->addSample((int8_t)*data);
        //     data++;
        // }
        // f->DFT();
        // DMESG("DFT DONE!");
        // f->processReal(); // Currently not working, don't know why.
        // f->processComplex();
        // DMESG("FFT DONE!");
        DMESG("TIME: %d", (int)(uBit.systemTime() - time));
        if (uBit.systemTime() - time >= RECVTIMEOUT)
        {
            DMESG("TIMEOUT");
            sampler->stop();
            break;
        }
        else
        {
            DMESG("STILL IN RECV");
        }
    }
    DMESG("GOING TO SEND");
    uBit.display.setBrightness(255);
    uBit.radio.enable();
    send();
}

// void test()
// {
//     DMESG("TEST");
//     PRINTFLOAT(1.234);
//     FFT *f = new FFT();

//     double data[] = {-69, -65, -72, -67, -65};
//     for (int i = 0; i < 5; i++)
//     {
//         f->addSample(data[i]);
//     }
//     f->DFT();
//     DMESG("DFT DONE!");
//     f->processComplex();
//     // f->processReal();
//     DMESG("FFT DONE!");
//     while (true)
//     {
//         fiber_sleep(200);
//     }
// }

std::string uint64_to_string(uint64_t value)
{
    std::ostringstream os;
    os << value;
    return os.str();
}

static void radioReceive(MicroBitEvent)
{
    uBit.display.print("J");
    PacketBuffer b = uBit.radio.datagram.recv();
    DMESG("MESSAGE FROM: %s", b.getBytes());
    uBit.radio.disable();
    recv();
}

int main()
{
    uBit.init();
    initialise();
    DMESG("INIT!");
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
            DMESG("A");
            send();
            break;
        }
        else if (uBit.buttonB.isPressed())
        {
            DMESG("B");
            recv();
            break;
        }
        // else if (uBit.logo.isPressed())
        // {
        //     test();
        //     break;
        // }
        uBit.sleep(500);
    }
}