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
int setup_error = 0;
int module_setup = 0;
int revision = -1;

extern int f_a20;

const int physToGpio_BP [64] =		//BOARD MODE
{
	-1, // 0
	-1, -1, //1, 2
	53, -1, //3, 4
	52, -1, //5, 6
	226, 228, //7, 8
	-1, 229, //9, 10
	275, 259, //11, 12
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
	257,256, //map to BCM GPIO0,1
	53,52, //map to BCM GPIO2,3
	226,35, //map to BCM GPIO4,5
	277,270, //map to BCM GPIO6,7
	266,269, //map to BCM GPIO8,9
	268,267, //map to BCM GPIO10,11
	276,45, //map to BCM GPIO12,13
	228,229, //map to BCM GPIO14,15
	38,275, //map to BCM GPIO16,17
	259,39, //map to BCM GPIO18,19
	44, 40, //map to BCM GPIO20,21
	273,244, //map to BCM GPIO22,23
	245,272, //map to BCM GPIO24,25
	37, 274, //map to BCM GPIO26,27
	-1,-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 28... 43
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, //44... 59
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


int is_valid_raw_port(int channel)
{
	if (channel >= 0 && channel < 18) return 1; // PA
	if (channel >= 32 && channel < 56) return 2; // PB
	if (channel >= 64 && channel < 89) return 3; // PC
	if (channel >= 96 && channel < 124) return 4; // PD
	if (channel >= 128 && channel < 140) return 5; // PE
	if (channel >= 160 && channel < 166) return 6; // PF
	if (channel >= 192 && channel < 204) return 7; // PG
	if (channel >= 224 && channel < 252) return 8; // PH
	if (channel >= 256 && channel < 278) return 9; // PI
	
	return 0;
}


int get_gpio_number(int channel, unsigned int *gpio,unsigned int *sys_gpio)
{
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
		} 
		else 
		{
			*gpio = *(*pin_to_gpio+channel);	//pin_to_gpio is initialized in py_gpio.c, the last several lines
			*sys_gpio = *(physToGpioR3 + channel);
		}
	}
	else if (gpio_mode == BCM)
	{
		*gpio = *(pinTobcm_BP + channel); 
		*sys_gpio = channel;
	}
	else if (gpio_mode == MODE_RAW)
	{
		if (!is_valid_raw_port(channel))
		{
			PyErr_SetString(PyExc_ValueError, "The channel sent does not exist");
			return 5;
		}
		*gpio = channel;
		
		unsigned int i;
		for (i = 0; i < 64; i++)
		{
			if (*(pinTobcm_BP + i) == channel)
			{
				*sys_gpio = i;
				break;
			}
		}
	}
	else 
	{
		// setmode() has not been run
		PyErr_SetString(PyExc_RuntimeError, "Please set pin numbering mode using GPIO.setmode(GPIO.BOARD) or GPIO.setmode(GPIO.BCM) or GPIO.setmode(GPIO.RAW)");
		return 3;
	}

	if(lemakerDebug)
		printf("GPIO = %d,sys_gpio = %d\n", *gpio,*sys_gpio);
	
	return 0;
}
