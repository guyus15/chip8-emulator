# CHIP-8 Emulator

A program written in C to emulate the CHIP-8 interpreter.

## Installation

This emulator uses SDL 2 to render graphics to the screen, so you must ensure that this is present on your machine
before compiling.

After cloning the repository, you can compile the emulator using `make` from within the repository's directory.

To compile the test file, run `make test` from within the cloned repository's directory.

## Usage

Before running the emulator, you will need a ROM to use. Many ROMs can be found [here](https://github.com/kripod/chip8-roms).

The emulator can be run with the following (assuming you are on a Linux system):

`./main <path to ROM here>`

The test file can be run with the following:

`./chip8_test`

---

See [this](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM) technical reference for more information about CHIP-8.
