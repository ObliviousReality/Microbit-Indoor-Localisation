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
    int v = 1 << (pitchVolume >> 5);
    pin->setAnalogValue(v);
    pin->setAnalogPeriodUs(1000000 / f);
    fiber_sleep(hiT);
    pin->setAnalogValue(0);
    fiber_sleep(loT);
}

void recv()
{
    DMESG("RECV");
    uBit.display.print("r");
    // FFT *f = new FFT(*uBit.audio.splitter->createChannel());
    // f->recording = true;
    MicSampler *sampler = new MicSampler(*uBit.audio.splitter->createChannel());
    sampler->start();
    while (true)
    {
        fiber_sleep(20);
        int lev = sampler->getLevel();
        if (lev > 0)
        {
            uBit.display.setBrightness(255);
            DMESG("CUR LEVEL: %d", lev);
        }
        else
        {
            uBit.display.setBrightness(0);
        }
    }
}

void send()
{
    uBit.display.clear();
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