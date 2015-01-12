#!/usr/bin/env python
import RPi.GPIO as GPIO
import time
from threading import Timer

PIN_NUM = 12
channel = 7
GPIO.setmode(GPIO.BOARD)
GPIO.setup(PIN_NUM,GPIO.OUT)

GPIO.output(PIN_NUM,True)
print "\n value_%d = %d\n" %(PIN_NUM,GPIO.input(PIN_NUM))

GPIO.setup(channel,GPIO.IN,GPIO.PUD_DOWN)
print "\n value_%d = %d\n" %(channel,GPIO.input(channel))


def makehigh():
	print "\n value_%d = %d\n" %(channel,GPIO.input(channel))
	GPIO.output(PIN_NUM,False)
	print "\n value_%d = %d\n" %(PIN_NUM,GPIO.input(PIN_NUM))
	
	

	
GPIO.wait_for_edge(channel, GPIO.RISING)
t = Timer(1,makehigh)
t.start()
