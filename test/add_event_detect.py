import RPi.GPIO as GPIO
import time

PIN_NUM = 12
GPIO.setmode(GPIO.BOARD)
GPIO.setup(PIN_NUM,GPIO.IN,GPIO.PUD_DOWN)

print "The value of Pin %d is %d" %(PIN_NUM,GPIO.input(PIN_NUM))


def my_callback(channel):
	print "Callback trigger %d" %channel
	print "Now value of the Pin is %d" %(GPIO.input(PIN_NUM))
	print "Click Ctr + C to exit"

GPIO.add_event_detect(PIN_NUM,GPIO.RISING,callback = my_callback,bouncetime = 300)

try:
    while True:
	time.sleep(0.1)
except KeyboardInterrupt:
    pass

GPIO.cleanup()

