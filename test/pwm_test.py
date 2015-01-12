#/usr/bin/env python

import RPi.GPIO as GPIO
import time

PIN_NUM = 11

GPIO.setmode(GPIO.BOARD)
GPIO.setup(PIN_NUM,GPIO.OUT)

p = GPIO.PWM(PIN_NUM,0.5)
p.start(50)
raw_input("Press Enter to stop:")
p.stop()
GPIO.cleanup()
