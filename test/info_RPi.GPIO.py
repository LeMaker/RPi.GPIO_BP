import RPi.GPIO as GPIO

ver_Bpi = GPIO.RPI_REVISION
ver_Gpio = GPIO.VERSION

print "BPi VERSION:\t\t",ver_Bpi
print "RPi.GPIO VERSION:\t",ver_Gpio
