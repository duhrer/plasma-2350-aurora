# Plasma 2350 Aurora

This project configures a Raspberry Pi Pico to control a 20 x 20 curtain of
WS2812 lights. It supports multiple modes, which are selectable using an IR
remote (see below).

In addition to displaying solid colours (including white), there are several
"aurora" modes which move strands of colour through the grid.

## Prerequisites

### Hardware

First, you need the right hardware:

1. An RP2040 or RP2035 microcontroller.  I used the [Pimoroni Plasma
   2350](https://shop.pimoroni.com/products/plasma-2350?variant=42092628246611),
   because it has a built-in boost converter for neopixels, and because it
   easily connects to strands of lights using screw terminals.
2. A curtain of 5V WS2821 LEDs.  I used [this model](https://nl.aliexpress.com/item/1005010382840396.html).
3. An IR receiver and remote (see below).

#### Infrared Receiver and Remote

Most of the functions used in this project are accessed using an IR receiver and
remote. I used [this IR
receiver](https://www.tinytronics.nl/en/communication-and-signals/wireless/infrared/ir-infrared-receiver-module-38khz-940nm)).
Connect the `ground` and `vcc` pins to the `ground` and `3v3` headers on your
microcontroller. Connect the `out` pin to the header for `GPIO5`. Note that this
component only works if it has enough of an unobstructed "line of sight" from
outside, so you'll need to design your case and/or mounting arrangement with
that in mind.

I use this project with [the remote from this
kit](https://www.tinytronics.nl/en/communication-and-signals/wireless/infrared/ir-sensor-module-with-remote-and-battery-with-ir-led)
and [the remote for the lights I
have](https://www.amazon.nl/-/en/Changing-Dimmable-Control-Colours-Decoration/dp/B06XYFZ4J5/ref=sr_1_1?sr=8-1).
This type of remote seems incredibly common in kits, and is also readily
available online.

### Software

Once you have something to run the code on, you'll need to set up a build
environment. In the past, I have used:

1. The [Getting Started with Pico Guide](https://datasheets.raspberrypi.org/pico/getting-started-with-pico.pdf)
2. The [Pico VS Code extension](https://github.com/raspberrypi/pico-vscode)

Both of those are probably the easiest starting points, and if you hit upon
questions, there are lots of people working with them, so hopefully you can find
the guidance you need.

Personally, I use [distrobox](https://distrobox.it/) and [the docker container
created by `lukstep`](https://github.com/lukstep/raspberry-pi-pico-docker-sdk),
which I set up using commands like:

```
distrobox-create --image lukstep/raspberry-pi-pico-sdk -n pico-sdk
distrobox enter pico-sdk
```

## Changing the Configuration

A few things are hard coded that you might want to change:

1. The GPIO pins used for the neopixel strands
2. The total number of lights.
3. The number of lights in each row and column.
4. The GPIO pin used for the IR receiver
5. The default brightness
6. The default "mode"

You can do change these by editing the compiler directives (`define` statements)
in `src/common_defaults.h` and `src/aurora.cpp`.

## Building and Installing

### VS Code and a PiProbe

If you have a
[PiProbe](https://www.raspberrypi.com/documentation/microcontrollers/debug-probe.html),
you should be able to use the debugger configuration in this project to build,
install (and debug) the code in this project.

First, you should check the paths in `.vscode/launch.json`and
`.vscode/settings.json` and update them as needed to match your system. Then,
you should be able to just hit the debugger icon and choose the configuration
defined in `.vscode/launch.json`.

The application will be built, deployed, and will pause execution at the
beginning of the `main()` function.

### Manual Build

To build the application from the command line, you can use commands like the
following, starting at the root the repository:

```
mkdir build
cd build
cmake ..
make -j16
```

The last command assumes you have sixteen cores, adjust as needed. Once the
build completes, there are two ways to install the application.

### Installing

This project builds a `plasma-aurora` binary, which you need to install on your
board. If you don't have a PiProbe, reboot your Pico while holding the "Bootsel"
button, then copy or drag the generated UF2 file `plasma-aurora.uf2` onto
the USB drive that appears.

If you're lucky enough to have a board with a reset button, all of the binaries
in this project also support entering `bootsel` mode by pressing the reset
button twice. You then copy the `plasma-aurora.uf2` file to the USB drive
as described above.

If you have a PiProbe, you can install the program without resetting your Pico
using a command like:

```
sudo openocd -f interface/cmsis-dap.cfg -f target/rp2350.cfg -c "adapter speed 5000" -c "program plasma-aurora.elf verify reset exit"
```

## Usage

Once you've put together the hardware and built and installed the binary, the
unit should start up in the default "aurora" mode. If you want to use any other
mode, you'll need to either change the defaults (see above) or have an IR
receiver and remote.

Most of the functions on the Practical Series II remote are supported, i.e. you
can turn the lights off and on, change the brightness level, and choose to either display a single colour or
to use one of the two "rainbow" modes. The first rainbow mode cycles through six
colours (the rainbow, basically, but I don't distinguish between indigo and
violet). The second is the default "double rainbow" mode described above.
