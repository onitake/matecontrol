@mainpage Documentation

@section intro Overview

The Matemat is an automatic beverage dispenser, designed to be open hardware.
It is based on a discarded C*ca C*la vending machine.

This is the documentation for the device firmware.


@section download Obtaining the source

The source code can be downloaded from the project's github page:
https://github.com/onitake/matecontrol

When cloning the git repository, make sure also get the source code for the
submodlues. The command `git submodule update --init` does this for you.

	
@section build Building the source

To compile the firmware, you need a current version of avr-gcc and avr-libc.
At least avr-gcc 4.6 is recommended. For the realtime clock, avr-libc 1.8.1
and avr-gcc 4.9 are required.


@section errata Errata

In controller board revision 1, the USB D+ and D- pins were swapped on the
USB B port. They need to be crossed or the USB-serial port does not work.
R3 and R5 should each be bridged, the 1k and DNP markings are wrong.
They connect the RX and TX lines.

USB_SLEEP is not connected in board revision 1. Adding a trace allows the
firmware to detect a USB connection.
