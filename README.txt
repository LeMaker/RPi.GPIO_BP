RPi.GPIO_BP
===========

This a modified version RPi.GPIO for Banana Pro. The original version for Raspberry Pi can be find from https://pypi.python.org/pypi/RPi.GPIO. The api fucntion isntruction can also refer to http://sourceforge.net/p/raspberry-gpio-python/wiki/Home/ .

************* How to install RPi.GPIO library for Banana Pro*************

1. sudo apt-get update
2. sudo apt-get install python-dev ==> I just try python 2.7.x, did not try 3.x.x
3. cd RPi.GPIO_BP
4. python setup.py install ===> Attention: do this step for /build folder is the user permission not root permission
5. sudo python setup.py install  ==> generate GPIO.so

The modified RPi.GPIO for Banana Pro is powered by LeMaker < http://www.lemaker.org/ > .
