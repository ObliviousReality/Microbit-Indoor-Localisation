#include "MicroBit.h"
#include "samples/Tests.h"
#include "MicSampler.h"
#include "fft.h"
#include <cstring>
// #include "global.h"

MicroBit uBit;
SoundOutputPin *pin = &uBit.audio.virtualOutputPin;

char prompts[] = {'<', 'S', '<', ' ', '>', 'R', '>', ' '};
int promptCounter = 0;
const int MAXPROMPTS = 8;

int radioRecv = 0;

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

void recv()
{
    DMESG("RECV");
    uBit.display.setBrightness(0);
    for (int y = 0; y < 5; y++)
    {
        for (int x = 0; x < 5; x++)
        {
            uBit.display.image.setPixelValue(x, y, 255);
        }
    }
    MicSampler *sampler = new MicSampler(*uBit.audio.splitter->createChannel());
    sampler->start();
    FFT *f = new FFT();
    double data[] = {-69, -65, -72, -67, -65};
    for (int i = 0; i < 5; i++)
    {
        f->addSample(data[i]);
    }
    f->DFT();
    DMESG("DFT DONE!");
    f->processComplex();
    // f->processReal();
    DMESG("FFT DONE!");
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
        // std::vector<double> vec;
        // for (int i = 0; i < buf->length(); i++)
        // {
        //     vec.push_back(abs((int8_t)*data));
        //     data++;
        // }
    }
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

void send()
{
    uBit.display.clear();
    DMESG("VOLUME: %d", uBit.audio.getVolume());
    uBit.radio.setGroup(5);
    char name[7];
    uint64_t serial = uBit.getSerialNumber();
    uint64_t val = serial;
    for (int i = 0; i < 6; i++)
    {
        name[i] = 65 + ((val / 2) % 26);
        val = (int)(val / 2);
    }
    name[6] = '\0';
    DMESG("%s", name);

    while (true)
    {
        if (uBit.buttonA.isPressed())
        {
            uBit.display.clear();
            while (true)
            {
                uBit.radio.datagram.send("hew");
                fiber_sleep(20);
                // playTone(8000, 20, 1000);
                if (uBit.buttonB.isPressed())
                {
                    uBit.display.print("x");
                    break;
                }
            }
        }
    }
}

static void radioReceive(MicroBitEvent)
{
    PacketBuffer b = uBit.radio.datagram.recv();
    DMESG("BYTES: %s", b.getBytes());
    DMESG("LENGTH: %d", b.length());
    // for (int i = 0; i < b.length(); i++)
    // {
    //     DMESG("%s", b[i]);
    // }
}

int main()
{
    uBit.init();
    DMESG("INIT!");
    uBit.messageBus.listen(DEVICE_ID_RADIO, MICROBIT_RADIO_EVT_DATAGRAM, radioReceive);
    uBit.radio.enable();
    uBit.radio.setGroup(5);
    while (true)
    {
        uBit.display.print(prompts[promptCounter++]);
        if (promptCounter > MAXPROMPTS)
        {
            promptCounter = 0;
        }
        // if (uBit.buttonAB.isPressed())
        // {
        //     test();
        //     break;
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
        // else if (uBit.logo.isPressed())
        // {
        //     test();
        //     break;
        // }
        uBit.sleep(500);
    }
}