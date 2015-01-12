#!/usr/bin/env python
import RPi.GPIO as GPIO
import time
PIN_NUM = 18

GPIO.setmode(GPIO.BCM)
GPIO.setup(PIN_NUM,GPIO.OUT)

GPIO.output(PIN_NUM,True)
print "\n value = %d\n" %(GPIO.input(PIN_NUM))	
time.sleep(1)
GPIO.output(PIN_NUM,False)
print "\n value = %d\n" %(GPIO.input(PIN_NUM))
time.sleep(1)

GPIO.cleanup()
