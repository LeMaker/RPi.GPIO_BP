import RPi.GPIO as GPIO
import time

PIN_NUM = 12
GPIO.setmode(GPIO.BOARD)
GPIO.setup(PIN_NUM,GPIO.OUT)

frequency = 50
p = GPIO.PWM(PIN_NUM,frequency)
p.start(0)

try:
    while True:
        for dutyCycle in range(0,100,5):
            p.ChangeDutyCycle(dutyCycle)
            time.sleep(0.1)
        for dutyCycle in range(100,0,-5):
            p.ChangeDutyCycle(dutyCycle)
            time.sleep(0.1)
except KeyboardInterrupt:
    pass

p.stop()
GPIO.cleanup()
