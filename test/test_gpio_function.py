import RPi.GPIO as GPIO

validPins = [11,12,13,15]

GPIO.setmode(GPIO.BOARD)

GPIO.setup(11,GPIO.IN)
GPIO.setup(12,GPIO.OUT)
GPIO.setup(13,GPIO.IN)
GPIO.setup(15,GPIO.OUT)

for pin in validPins:
	fun = GPIO.gpio_function(pin)
	print fun
