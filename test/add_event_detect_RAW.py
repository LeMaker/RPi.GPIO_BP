import RPi.GPIO as GPIO 
import time

#PIN_NUM = 24
#GPIO.setmode(GPIO.BOARD)
#GPIO.setup(PIN_NUM,GPIO.IN,GPIO.PUD_DOWN)

GPIO.setmode(GPIO.RAW)
GPIO.setup(GPIO.PI+10,GPIO.IN,GPIO.PUD_DOWN)

print "The value of Pin %d is %d" %(GPIO.PI+10,GPIO.input(GPIO.PI+10))


def my_callback(channel):
	print "Callback trigger %d" %channel
	print "Now value of the Pin is %d" %(GPIO.input(GPIO.PI+10))
	print "Click Ctr + C to exit"

GPIO.add_event_detect(GPIO.PI+10,GPIO.RISING,callback = my_callback,bouncetime = 300)

try:
    while True:
	time.sleep(0.1)
except KeyboardInterrupt:
    pass

GPIO.cleanup()

