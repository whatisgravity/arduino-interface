# Arduino Interface

Arduino interface was created because I hate programming arduinos. A GUI IDE seems to be required to easily send software to the arduino.

Arduino interface is minimal but a highly flexible async/event driven arduino program designed to push all the logic off the arduino and onto a computer connected to the arduino by serial.

This only works with projects where the Arduino is connected to a
computer by USB.

* Reads JSON statements from the serial port to set pins, and change the value
  of output pins.

* Writes JSON statements to the serial port that contain the state
  information of each pin.

Doing this allows every arduino that are connected to computers by USB to be flashed with the same
standardized software with a standarized way to interface with every
pin, regardless of the type of Arduino device.

## Server side

Included is a golang program and the corresponding compiled binary that
interfaces with the arduino over the serial port. It also starts a
webserver that has both a REST API and a WebSockets API. This allows easy remote interfacing with the arduino.

This is based on a previous project to control lights and so there are
two mock up arduino-interface server and clients written in Ruby
included.

The golang client is not finished but it planned.

## Interface directly with arduino

To interface directly with the arduino over serial you first must
initialize a port:

    {"setPins":[{"number": 5, "output": true, "analog": true}]}

You can set more than one pin at a time:

    {"setPins":[{"number": 5, "output": true, "analog": true},
{"number": 2, "output": true, "analog": true}]}

After setting a pin you can change the value:

## Installation of included tools

An install script has been included to simplyify installation but the basic instructions are below:

    ./INSTALL.sh

Alternatively, you can manually install:

    cp actrl /usr/local/bin
    cp asrvr /usr/local/bin

Then you copy the config file to your home directory, update the config to suite your needs

    cp config/config.sample.yml ~/.lctrl.yml
    vim ~/.lctrl.yml

Then start the server

    lsrvr -s desk -d

Then you can use the ltctrl command to control the lights. 

## Usage with included tools

For the command line interface:

    Usage:
    Please specify colors by name, json or rgb:
        lctrl [options]
    Options:
        --server [option]       # Specify the server from the configuration file
        --red    [option]       # Provide a number 0-255, 0 being off and 255 being the brightest
        --green  [option]       # Provide a number 0-255, 0 being off and 255 being the brightest
        --blue   [option]       # Provide a number 0-255, 0 being off and 255 being the brightest
        --color  [option]       # Select between, "red", "blue", "green" or "white"
        --json   [json]         # Supply json directly: {"red": 0, "blue": 255, "green", 0}
        --random                # Randomly select a color
        --off                   # Turns off the lights
    Example:
        lctrl --green 255 --blue 24
        lctrl -g 255 -b 24


For the server daemon:

    Usage:
    Please specify the server defined in the configuration to start:
        asrvr [options]
    Options:
        --server [option]       # Provide a server defined in the config file
        --daemon                # Daemonize the server
    Example:
        asrvr --server wall
        asrvr -s wall

## Roadmap

If I have time in the future I would like to add more configuration options, you should be able to specify if you would like to send the message directly over serial and what port, or over websockets, or over TCP.
