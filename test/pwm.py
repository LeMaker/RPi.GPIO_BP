#!/usr/bin/env python
import RPi.GPIO as GPIO
import time
LED_PIN = 12

GPIO.setmode(GPIO.BOARD)
GPIO.setup(LED_PIN,GPIO.OUT)
pwm = GPIO.PWM(LED_PIN, 50)
pwm.start(100)
print "\nPWM tests"
response = raw_input('Is the LED on (y/n) ? ').upper()
self.assertEqual(response,'Y')
pwm.start(0)
response = raw_input('Is the LED off (y/n) ? ').upper()
self.assertEqual(response,'Y')

print "LED Brighten/fade test..."
for i in range(0,3):
	for x in range(0,101,5):
		pwm.ChangeDutyCycle(x)
		time.sleep(0.1)
	for x in range(100,-1,-5):
		pwm.ChangeDutyCycle(x)
		time.sleep(0.1)
pwm.stop()
response = raw_input('Did it work (y/n) ? ').upper()
self.assertEqual(response,'Y')
GPIO.cleanup()

