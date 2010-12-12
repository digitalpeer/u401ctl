u401ctl
----------------------------------------------------------------------------
This is a simple utility which uses libusb to control a U401 USB device.
It currently allows you to use PORTA of the chip as outputs and individually 
control control those outputs. 

This is useful, for example, to script controlling a U401 and turn specific 
outputs on and off.

More information about the device can be found at http://usbmicro.com/.


Compiling
----------------------------------------------------------------------------
Just run make and hopefully you'll end up with a u401ctl binary.


TODO
----------------------------------------------------------------------------
 - Add support for PORTB.
 - Add reading inputs, not just settings outputs.
