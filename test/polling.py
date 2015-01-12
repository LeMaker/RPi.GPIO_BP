import RPi.GPIO as GPIO
import time

channel = 7

GPIO.setmode(GPIO.BOARD)
GPIO.setup(channel,GPIO.IN,GPIO.PUD_DOWN)
print " Now input is Low\n Our task is performed when it becomes High"

while True:
    if GPIO.input(channel):
        print "Input was High,begin to perform"
	print "Count Down"
        for i in range(7,0,-1):
            print "%d" %i
            time.sleep(1)
        print "Performed!"
	exit()
