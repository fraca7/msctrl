
# msctrl

This little project aims to provide a way to control the Sega Master System console with modern game controllers (DualShock, etc).

## Hardware

  * A Raspberry Pi Zero W. Anything with enough GPIO outputs and running Linux is fine actually; Bluetooth is a nice addition.
  * A breadboard
  * 6 transistors and some cables
  * A genuine Master System controller to provide the connector.
  * A soldering iron and pump

## Wiring

First, unscrew the Master System controller and unsolder the cables. Each one has a distinct color:

  * 1 (Up) is brown
  * 2 (Down) is red
  * 3 (Left) is orange
  * 4 (Right) is yellow
  * 6 (B1) is blue
  * 8 (GND) is black
  * 9 (B2) is white

If you want to use your own connector instead of one ripped from a genuine controller, follow the pinout:

![Master System connector pinout](/doc/sms_joystick.gif)

Next use the breadboard to connect everything; appart from 8/black which should be connected to the RPi's GND, each RPi GPIO is connected to a transistor's base like this:

![GPIO to base, collector to ground, emitter to controller pin](/doc/transistor.png)

The GPIO to pin mapping is thus (it can be changed in the source though):

  * 1 (brown) to GPIO2
  * 2 (red) to GPIO3
  * 3 (orange) to GPIO4
  * 4 (yellow) to GPIO17
  * 6 (blue) to GPIO27
  * 9 (white) to GPIO22

The end result should look like this (YMMV)

![Picture of my breadboard connected](/doc/breadboard.jpg)

## Building

Once the OS is installed, check out the source on the RPi. You'll need the following libraries installed:

  * build-essential
  * cmake
  * libfmt-dev
  * libsdl2-dev
  * nlohmann-json3-dev
  * libspdlog-dev
  * libpigpio-dev

Then build

```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

## Running

### Pairing a controller

For the DualShock and DualSense, you can enter pairing mode by holding PS and Share until the LED flashes.

If you have installed a graphic environment, use whatever Bluetooth manager it has to pair your gamepad to the RPi. If you want to pair it from the command lie, you can use bluetooth-ctl:

```
bluetooth-ctl scan on
```

As soon as your gamepad shows up in the output, copy its MAC address and issue

```
bluetooth-ctl trust <MAC>
bluetooth-ctl pair <MAC>
```

### Gyro calibration

If your gamepad has a gyro and you intend to use it (see below) it may need calibration. The gamepad must be completely still for a few seconds after the executable is run, until you see something like this in the console:

```
[2023-10-30 11:44:59.001] [info] PS4 Controller (+X) calibrated: -0.002
```

(this message will only be displayed if you configure a gyro mapping, see below)

### Permissions

Strangely enough even if the user is part of the **gpio** group, initialization fails on the last Raspberry Pi OS. Just launch the program using sudo.

### Usage

You can specify any button mapping on the command line. Here is a description of all options (you can get a summary using *-h*)

#### Mapping buttons

The *-b* option lets you specify which button on the gamepad will map to which on the Master System. Gamepad button names are the ones used by SDL so they don't depend on your exact model of gamepad; here are the mappings for a DualShock for instance:

  * A (Cross)
  * B (Circle)
  * X (Square)
  * Y (Triangle)
  * LS (L1) - Left Shoulder
  * RS (R1) - Right Shoulder
  * LT (L2) - Left Trigger
  * RT (R2) - Right Trigger

The Master System only supports two buttons:

  * B1
  * B2

The syntax is

```
-b <gamepad button>:<console button>
```

The option can be specified multiple times. So, to map both A and B to B1:

```
./msctrl -b A:B1 -b B:B1
```

Triggers (LT and RT) are a special case since they actually report a value (normalized between 0 and 1), not a state. You can adjust the trigger sensitivity using the *-t* option; the default is 0.5. To make them more sensitive use a lower value:

```
./msctrl -t 0.1 RT:B1
```

#### Mapping DPad

The *-d* option enables mapping from the gamepad's digital pad to the console's pad.

#### Mapping sticks

Analog sticks can be mapped to the console's pad as well, using the *-s* option. In its simplest form you just specify which stick should be mapped (L for left, R for right):

```
./msctrl -s L
```

Since sticks are analog there are a number of parameters you can set to tune how it works. Sticks have a deadzone and some values to fine-tune the hysteresis used internally to avoid repeated press/release events when they're right around the threshold. For instance

```
./msctrl -s L,lo=0.1,hi=0.15,ht=1
```

will use a smaller deadzone and angle thresholds. In general you can go with the defaults.

#### Mapping the gyro

Each gyro axis can be mapped to two pad buttons independently. There are two main parameters for a gyro mapping: the angle threshold, and the trigger buttons.

  * The angle threshold is the minimal inclination the gamepad must have in order to trigger a button press
  * The trigger buttons are the way to enable motion controls. The mapping will only be active when all trigger buttons are pressed simultaneously.

In addition, the inclination of the gamepad is relative to its absolute position when the triggers buttons were pressed.

For instance, to map the Z axis to left/right (for AfterBurner for instance), using a threshold of 15 degrees (default is 20), when the right shoulder button is pressed:

```
./msctrl -g +Z:L,th=15,RS -g -Z:R,th=15,RS
```

Axis may be *+X*, *-X*, *+Y*, *-Y*, *+Z* or *-Z* (the "+" part is actually optional). Target buttons are R,L,U,D (right, left, up, down).

### Saving and loading configurations

Instead of specifying everything on the command line each time, you can use the *-o* option to save the current configuration to a JSON file:

```
./msctrl -g +Z:L,th=15 -g -Z:R,th=15 -g +X:U,th=15 -g -X:D,th=15 -b A:B1 -b RT:B2 -o configuration.json
```

You can then later load the configuration using the *-c* option:

```
./msctrl -c configuration.json
```

You can fine-tune an existing configuration by specifying additional mappings **after** the *-c*:

```
./msctrl -c configuration.json -b B:B2
```
