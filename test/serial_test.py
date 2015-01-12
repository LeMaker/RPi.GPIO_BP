import serial,time
import RPi.GPIO as GPIO

ser = serial.Serial('/dev/ttyS1',115200,timeout = 1)
#ser.open()

print ser.portstr

strInput = raw_input("Enter some words:")
num = ser.write(strInput)
print num

time.sleep(1)
str = ser.read(num)#ser.readall()
print str
ser.close()

#try:
#    while True:
#        response = ser.readline()
#        print response
#except KeyboardInterrupt:
#    ser.close()

