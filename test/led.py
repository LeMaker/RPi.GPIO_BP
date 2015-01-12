#!/usr/bin/env python
import RPi.GPIO as GPIO
import time
PIN_NUM = 7

GPIO.setmode(GPIO.BCM)
#GPIO.setmode(GPIO.BOARD)
GPIO.setup(PIN_NUM,GPIO.OUT)
while True:

	GPIO.output(PIN_NUM,True)
	
	time.sleep(1)
	GPIO.output(PIN_NUM,False)
	time.sleep(1)

