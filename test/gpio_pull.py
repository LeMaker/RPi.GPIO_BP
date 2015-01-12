#!/usr/bin/env python
import RPi.GPIO as GPIO
import time

GPIO.setmode(GPIO.BOARD)
GPIO.setup(7, GPIO.IN, pull_up_down=GPIO.PUD_UP)
print "\n value_7 = %d\n" %(GPIO.input(7))	

GPIO.setup(8, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
print "\n value_8 = %d\n" %(GPIO.input(8))	

GPIO.setup(12, GPIO.IN, pull_up_down=GPIO.PUD_UP)
print "\n value_12 = %d\n" %(GPIO.input(12))	
GPIO.setup(12, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
print "\n value_12 = %d\n" %(GPIO.input(12))	

#GPIO.cleanup()
