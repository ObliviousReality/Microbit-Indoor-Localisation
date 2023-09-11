# Microbit Indoor Localisation

A project that attempts to accurately calculate the distance between two Microbit V2s using Audio and Radio signals.

This code is accurate in open, uninterrupted spaces to the nearest 10cm, sometimes better, but with a maximum range of around 1.5-2 meters.

No avenue is provided to digitally display this distance on a Microbit, instead the readouts can be found on a HTML CSV file stored within the Microbit.

Additional information may be learned from connecting one of the Microbits to a PC and monitoring the serial communications.

# Installation
(Taken from the parent repository)

You need some open source pre-requisites to build this repo

- [GNU Arm Embedded Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)
- [Github desktop](https://desktop.github.com/)
- [CMake](https://cmake.org/download/)
- [Python 3](https://www.python.org/downloads/)

We use Ubuntu Linux for most of our tests. You can also install these tools easily through the package manager:

(Alternatively, on Windows WSL may be used instead)

```
    sudo apt install gcc
    sudo apt install git
    sudo apt install cmake
    sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi
```

# Building
- Clone this repository
- In the root of this repository type `python build.py`
- The hex file will be built `MICROBIT.HEX` and placed in the root folder.

