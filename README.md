RPi.GPIO_BP README

This is a modified version RPi.GPIO for Banana Pro/Pi. We call it RPi.GPIO_BP.
It is based on the original [RPi.GPIO](https://pypi.python.org/pypi/RPi.GPIO).
The modification is done by [LeMaker](http://lemaker.org). The RPi.GPIO_BP API usage are the same to the original RPi.GPIO.
You can donwload the RPi.GPIO_BP from:
https://github.com/LeMaker/RPi.GPIO_BP.

## Download
### For Banana Pro
    git clone https://github.com/LeMaker/RPi.GPIO_BP -b bananapro
### For Banana Pi
    git clone https://github.com/LeMaker/RPi.GPIO_BP -b bananapi
## Installation
    sudo apt-get update
    sudo apt-get install python-dev
    cd /RPi.GPIO_BP
    python setup.py install                 
    sudo python setup.py install
    
Please be attention that you need use both python and sudo pytohn to make the RPi.GPIO_BP work well.

## Examples
You can go to LeMaker wiki to see the basic examples: http://wiki.lemaker.org/RPi.GPIO

And the source directory test also has many demo.

## Extra
This version supports a new addressing mode "RAW" which enables you to use any GPIO pin. Below is an example which sets PD10 (which is pin 29 on the LCD connector) to a high level.

    import RPi.GPIO as GPIO
    GPIO.setmode(GPIO.RAW)
    GPIO.setup(GPIO.PD+10, GPIO.OUT)
    GPIO.output(GPIO.PD+10, 1)


Thanks!

LeMaker Team
