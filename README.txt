RPi.GPIO_BP
===========

This a modified version RPi.GPIO for Banana Pi. The original version for Raspberry Pi can be find from https://pypi.python.org/pypi/RPi.GPIO. The api fucntion isntruction can also refer to http://sourceforge.net/p/raspberry-gpio-python/wiki/Home/ .

************* How to install RPi.GPIO library for Banana Pi*************

1. sudo apt-get update
2. sudo apt-get install python-dev ==> I just try python 2.7.x, did not try 3.x.x
3. cd RPi.GPIO_BP
4. python setup.py install ===> Attention: do this step for /build folder is the user permission not root permission
5. sudo python setup.py install  ==> generate GPIO.so

The modified RPi.GPIO for Banana Pi is powered by LeMaker < http://www.lemaker.org/ > .

This version supports a new addressing mode "RAW" which enables you to use any GPIO pin. Below is an example which sets PD10 (which is pin 29 on the LCD connector) to a high level.

import RPi.GPIO as GPIO
GPIO.setmode(GPIO.RAW)
GPIO.setup(GPIO.PD+10, GPIO.OUT)
GPIO.output(GPIO.PD+10, 1)