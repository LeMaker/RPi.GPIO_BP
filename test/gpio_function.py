import RPi.GPIO as GPIO

validPins = [3,5,8,7,10, 12,11,13,16,15, 18,19,22,21,24 ,23,26]

GPIO.setmode(GPIO.BOARD)

for pin in validPins:
	fun = GPIO.gpio_function(pin)
	print fun

