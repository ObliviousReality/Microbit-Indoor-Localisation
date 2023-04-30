#include "MicroBit.h"
#include "samples/Tests.h"
#include "MicSampler.h"

MicroBit uBit;
SoundOutputPin *pin = &uBit.audio.virtualOutputPin;
uint8_t pitchVolume = 0xff;

char prompts[] = {'<', 'S', '<', ' ', '>', 'R', '>', ' '};
int promptCounter = 0;
const int MAXPROMPTS = 8;

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
        ManagedBuffer *buf = sampler->getBuffer();
        int16_t *data = (int16_t *)&buf[0];
    }
}

void send()
{
    uBit.display.clear();
    DMESG("VOLUME: %d", uBit.audio.getVolume());
    while (true)
    {
        if (uBit.buttonA.isPressed())
        {
            uBit.display.clear();
            while (true)
            {
                playTone(8000, 20, 1000);
                if (uBit.buttonB.isPressed())
                {
                    uBit.display.print("x");
                    break;
                }
            }
        }
    }
}

int main()
{
    uBit.init();
    while (true)
    {
        uBit.display.print(prompts[promptCounter++]);
        if (promptCounter > MAXPROMPTS)
        {
            promptCounter = 0;
        }
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