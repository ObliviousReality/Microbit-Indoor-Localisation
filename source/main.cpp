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
    FFT *f = new FFT();
    for (int y = 0; y < 5; y++)
    {
        for (int x = 0; x < 5; x++)
        {
            uBit.display.image.setPixelValue(x, y, 255);
        }
    }

    sampler->start();

    // double data[] = {-69, -65, -72, -67, -65};
    // for (int i = 0; i < 5; i++)
    // {
    //     f->addSample(data[i]);
    // }
    DMESG("LOOP START");
    long time = uBit.systemTime();
    while (true)
    {
        DMESG("LOOP");
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
        ManagedBuffer buf = sampler->getBuffer();
        int16_t *data = (int16_t *)&buf[0];
        f->clearSamples();
        for (int i = 0; i < 128; i++)
        {
            f->addSample((int8_t)*data);
            data++;
        }
        DMESG("LENGTH: %d", f->getSampleNumber());
        // f->DFT();
        // DMESG("DFT DONE!");
        f->processReal();
        // f->processComplex();
        DMESG("FFT DONE!");
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

void test()
{
    DMESG("TEST");
    fiber_sleep(20);
    FFT *f = new FFT();

    double data[] = {
        -8.0,   -9.0,   -9.0,   -9.0,   -8.0,   -8.0,   -8.0,   -8.0,   -8.0,   -9.0,  -8.0,
        -8.0,   -8.0,   -8.0,   -8.0,   -8.0,   -8.0,   -8.0,   -8.0,   -9.0,   -8.0,  -8.0,
        -9.0,   -8.0,   -8.0,   -8.0,   -8.0,   -9.0,   -9.0,   -8.0,   -9.0,   -8.0,  -8.0,
        -9.0,   -8.0,   -8.0,   -8.0,   -8.0,   -7.0,   -8.0,   -8.0,   -9.0,   -8.0,  -9.0,
        -8.0,   -8.0,   -9.0,   -8.0,   -7.0,   -8.0,   -8.0,   -8.0,   -8.0,   -8.0,  -9.0,
        -8.0,   -9.0,   -9.0,   -9.0,   -8.0,   -8.0,   -8.0,   -8.0,   -8.0,   -9.0,  -8.0,
        -8.0,   -8.0,   -8.0,   -8.0,   -8.0,   -8.0,   -8.0,   -8.0,   -9.0,   -8.0,  -8.0,
        -9.0,   -8.0,   -8.0,   -8.0,   -8.0,   -9.0,   -9.0,   -8.0,   -9.0,   -8.0,  -8.0,
        -9.0,   -8.0,   -8.0,   -8.0,   -8.0,   -7.0,   -8.0,   -8.0,   -9.0,   -8.0,  -9.0,
        -8.0,   -8.0,   -9.0,   -8.0,   -7.0,   -8.0,   -8.0,   -8.0,   -8.0,   -8.0,  -9.0,
        -8.0,   -8.0,   -9.0,   -8.0,   -7.0,   -8.0,   -7.0,   -9.0,   -8.0,   -7.0,  -8.0,
        -7.0,   -7.0,   -8.0,   -7.0,   -7.0,   -8.0,   -8.0,   -8.0,   -8.0,   -7.0,  -8.0,
        -8.0,   -8.0,   -8.0,   -8.0,   -8.0,   -7.0,   -7.0,   -7.0,   -8.0,   -7.0,  -8.0,
        -8.0,   -8.0,   -8.0,   -8.0,   -7.0,   -7.0,   -9.0,   -8.0,   -8.0,   -8.0,  -7.0,
        -8.0,   -8.0,   -8.0,   -8.0,   -7.0,   -8.0,   -7.0,   -7.0,   -7.0,   -8.0,  -8.0,
        -8.0,   -8.0,   -7.0,   -7.0,   -8.0,   -7.0,   -7.0,   -7.0,   -7.0,   -7.0,  -7.0,
        -7.0,   -7.0,   -7.0,   -7.0,   -7.0,   -6.0,   -7.0,   64.0,   0.0,    0.0,   0.0,
        0.0,    32.0,   5.0,    0.0,    0.0,    0.0,    0.0,    32.0,   0.0,    0.0,   0.0,
        34.0,   9.0,    0.0,    0.0,    0.0,    0.0,    32.0,   0.0,    0.0,    0.0,   34.0,
        0.0,    0.0,    0.0,    34.0,   0.0,    0.0,    0.0,    34.0,   17.0,   0.0,   0.0,
        0.0,    0.0,    32.0,   0.0,    0.0,    0.0,    34.0,   0.0,    0.0,    0.0,   34.0,
        0.0,    0.0,    0.0,    34.0,   0.0,    0.0,    0.0,    32.0,   0.0,    0.0,   0.0,
        32.0,   0.0,    0.0,    0.0,    32.0,   0.0,    0.0,    0.0,    32.0,   30.0,  0.0,
        -108.0, -108.0, -93.0,  -105.0, -101.0, -94.0,  -98.0,  -100.0, -89.0,  -96.0, -100.0,
        -104.0, -97.0,  -102.0, -104.0, -85.0,  -94.0,  -104.0, -104.0, -97.0,  -90.0, -98.0,
        -96.0,  -103.0, -101.0, -106.0, -100.0, -97.0,  -104.0, -93.0,  -103.0, -98.0, -106.0,
        -102.0, -97.0,  -99.0,  -89.0,  -94.0,  -106.0, -98.0,  -104.0, -91.0,  -98.0, -98.0,
        -104.0, -97.0,  -104.0, -101.0, -96.0,  -99.0,  -96.0,  -106.0, -107.0, -88.0, -101.0,
        -102.0, -94.0,  -104.0};
    for (int i = 0; i < 128; i++)
    {
        f->addSample(data[i]);
    }
    long st = uBit.systemTime();
    // f->DFT();
    long et = uBit.systemTime();
    // std::vector<std::complex<double>> *out = f->getDFTOutput();
    // for (int i = 0; i < f->getSampleNumber(); i++)
    // {
    //     PRINTFOURFLOAT(i, data[i], out->at(i).real(), out->at(i).imag());
    // }

    DMESG("DFT DONE!");

    DMESG("PROCESSING TIME: %d", (int)(et - st));
    st = uBit.systemTime();
    // f->processComplex();
    f->processReal();
    et = uBit.systemTime();
    DMESG("FFT DONE!");
    DMESG("PROCESSING TIME: %d", (int)(et - st));
    while (true)
    {
        fiber_sleep(200);
    }
}

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