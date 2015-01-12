/*
Copyright (c) 2013-2014 Ben Croston

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "Python.h"
#include "c_gpio.h"
#include "common.h"

int gpio_mode = MODE_UNKNOWN;
const int pin_to_gpio_rev1[27] = {-1, -1, -1, 0, -1, 1, -1, 4, 14, -1, 15, 17, 18, 21, -1, 22, 23, -1, 24, 10, -1, 9, 25, 11, 8, -1, 7};
const int pin_to_gpio_rev2[27] = {-1, -1, -1, 2, -1, 3, -1, 4, 14, -1, 15, 17, 18, 27, -1, 22, 23, -1, 24, 10, -1, 9, 25, 11, 8, -1, 7};
const int physToGpio_BP [64] =		//BOARD MODE
{
	-1, // 0
	-1, -1, //1, 2
	53, -1, //3, 4
	52, -1, //5, 6
	259, 224, //7, 8  (8--228)
	-1, 225, //9, 10
	275, 226, //11, 12
	274, -1, //13, 14
	273, 244, //15, 16
	-1, 245, //17, 18
	268, -1, //19, 20
	269, 272, //21, 22
	267, 266, //23, 24
	-1, 270, //25, 26
	
	257, 256, //27, 28
	35, -1, //29, 30
	277, 276, //31, 32
	45, -1, //33, 34
	39, 38, //35, 36
	37, 44, //37, 38
	-1, 40, //39, 40
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, //41-> 55
	-1, -1, -1, -1, -1, -1, -1, -1 // 56-> 63
} ;
const int pinTobcm_BP [64] =	//BCM MODE
{
	-1,-1, //map to BCM GPIO0,1
	53,52, //map to BCM GPIO2,3
	259,-1, //map to BCM GPIO4,5
	-1,270, //map to BCM GPIO6,7
	266,269, //map to BCM GPIO8,9
	268,267, //map to BCM GPIO10,11
	-1,-1, //map to BCM GPIO12,13
	224,225, //map to BCM GPIO14,15
	-1,275, //map to BCM GPIO16,17
	226,-1, //map to BCM GPIO18,19
	-1, -1, //map to BCM GPIO20,21
	273,244, //map to BCM GPIO22,23
	245,272, //map to BCM GPIO24,25
	-1, 274, //map to BCM GPIO26,27
	
	-1,-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 29... 44
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, //45... 60
	-1, -1, -1, -1 // ...63
} ;
const int physToGpioR3 [64] =//head num map to BCMpin
{
	-1, // 0
	-1, -1, // 1, 2
	2, -1,
	3, -1,
	4, 14,
	-1, 15,
	17, 18,
	27, -1,
	22, 23,
	-1, 24,
	10, -1,
	9, 25,
	11, 8,
	-1, 7, // 25, 26
	
	0, 1, //27, 28
	5, -1, //29, 30
	6, 12, //31, 32
	13, -1, //33, 34
	19, 16, //35, 36
	26, 20, //37, 38
	-1, 21, //39, 40
	// Padding:
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // ... 56
	-1, -1, -1, -1, -1, -1, -1, // ... 63
} ;

int setup_error = 0;
int module_setup = 0;
int revision = -1;

extern int f_a20;

int check_gpio_priv(void)
{
    // check module has been imported cleanly
    if (setup_error)
    {
        PyErr_SetString(PyExc_RuntimeError, "Module not imported correctly!");
        return 1;
    }

    // check mmap setup has worked
    if (!module_setup)
    {
        PyErr_SetString(PyExc_RuntimeError, "No access to /dev/mem.  Try running as root!");
        return 2;
    }
    return 0;
}

int get_gpio_number(int channel, unsigned int *gpio)
{
    // check setmode() has been run
    if (gpio_mode != BOARD && gpio_mode != BCM)
    {
        PyErr_SetString(PyExc_RuntimeError, "Please set pin numbering mode using GPIO.setmode(GPIO.BOARD) or GPIO.setmode(GPIO.BCM)");
        return 3;
    }

    // check channel number is in range
    if ( (gpio_mode == BCM && (channel < 0 || channel > 64))
      || (gpio_mode == BOARD && (channel < 1 || channel > 64)) )
    {
        PyErr_SetString(PyExc_ValueError, "The channel sent is invalid on a Banana Pi");
        return 4;
    }

    // convert channel to gpio
    if (gpio_mode == BOARD)
    {
    if (*(*pin_to_gpio+channel) == -1)
        {
            PyErr_SetString(PyExc_ValueError, "The channel sent is invalid on a Banana Pi");
            return 5;
        } else {
            *gpio = *(*pin_to_gpio+channel);	//pin_to_gpio is initialized in py_gpio.c, the last several lines
        }
    }
    else // gpio_mode == BCM
    {
          *gpio = *(pinTobcm_BP + channel); 
    }
    
	if(lemakerDebug)
		printf("GPIO = %d\n", *gpio);
    return 0;
}
