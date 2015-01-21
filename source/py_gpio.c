/*
Copyright (c) 2012-2014 Ben Croston

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
#include "event_gpio.h"
#include "py_pwm.h"
#include "cpuinfo.h"
#include "constants.h"
#include "common.h"

static PyObject *rpi_revision;
static int gpio_warnings = 1;

struct py_callback
{
   unsigned int gpio;
   PyObject *py_cb;
   struct py_callback *next;
};
static struct py_callback *py_callbacks = NULL;

static int mmap_gpio_mem(void)
{
   int result;

   if (module_setup)
      return 0;

   result = setup();
   if (result == SETUP_DEVMEM_FAIL)
   {
      PyErr_SetString(PyExc_RuntimeError, "No access to /dev/mem.  Try running as root!");
      return 1;
   } else if (result == SETUP_MALLOC_FAIL) {
      PyErr_NoMemory();
      return 2;
   } else if (result == SETUP_MMAP_FAIL) {
      PyErr_SetString(PyExc_RuntimeError, "Mmap of GPIO registers failed");
      return 3;
   } else { // result == SETUP_OK
      module_setup = 1;
		//printf("SETUP_OK!\n");
      return 0;
   }
}

// python function cleanup(channel=None)
static PyObject *py_cleanup(PyObject *self, PyObject *args, PyObject *kwargs)
{
   int i;
   int found = 0;
   int channel = -666;
   unsigned int gpio;
   static char *kwlist[] = {"channel", NULL};

   if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|i", kwlist, &channel))
      return NULL;

   if (channel != -666 && get_gpio_number(channel, &gpio))
      return NULL;

   if (module_setup && !setup_error) {
      if (channel == -666) {
         // clean up any /sys/class exports
         event_cleanup_all();

         // set everything back to input
         for (i=0; i<64; i++) {
            if (gpio_direction[i] != -1) {
               setup_gpio(i, INPUT, PUD_OFF);
               gpio_direction[i] = -1;
               found = 1;
            }
         }
      } else {
         // clean up any /sys/class exports
         event_cleanup(gpio);

         // set everything back to input
         if (gpio_direction[gpio] != -1) {
            setup_gpio(gpio, INPUT, PUD_OFF);
            gpio_direction[i] = -1;
            found = 1;
         }
      }
   }

   // check if any channels set up - if not warn about misuse of GPIO.cleanup()
   if (!found && gpio_warnings) {
      //PyErr_WarnEx(NULL, "No channels have been set up yet - nothing to clean up!  Try cleaning up at the end of your program instead!", 1);
   }

   Py_RETURN_NONE;
}

// python function setup(channel, direction, pull_up_down=PUD_OFF, initial=None)
static PyObject *py_setup_channel(PyObject *self, PyObject *args, PyObject *kwargs)
{
   unsigned int gpio;
   int channel, direction;
   int pud = PUD_OFF + PY_PUD_CONST_OFFSET;
   int initial = -1;
   static char *kwlist[] = {"channel", "direction", "pull_up_down", "initial", NULL};
   int func;

   if (!PyArg_ParseTupleAndKeywords(args, kwargs, "ii|ii", kwlist, &channel, &direction, &pud, &initial))
      return NULL;

   // check module has been imported cleanly
   if (setup_error)
   {
      PyErr_SetString(PyExc_RuntimeError, "Module not imported correctly!");
      return NULL;
   }

   if (get_gpio_number(channel, &gpio))
      return NULL;

   if (direction != INPUT && direction != OUTPUT)
   {
      PyErr_SetString(PyExc_ValueError, "An invalid direction was passed to setup()");
      return NULL;
   }

   if (direction == OUTPUT)
      pud = PUD_OFF + PY_PUD_CONST_OFFSET;

   pud -= PY_PUD_CONST_OFFSET;
   if (pud != PUD_OFF && pud != PUD_DOWN && pud != PUD_UP)
   {
      PyErr_SetString(PyExc_ValueError, "Invalid value for pull_up_down - should be either PUD_OFF, PUD_UP or PUD_DOWN");
      return NULL;
   }

   if (mmap_gpio_mem())
      return NULL;

   func = gpio_function(gpio);//!!!! has not modify 
#ifdef BAPI_DEBUG
   if (gpio_warnings &&                             // warnings enabled and
       ((func != 0 && func != 1) ||                 // (already one of the alt functions or
       (gpio_direction[gpio] == -1 && func == 1)))  // already an output not set from this program)
   {
      PyErr_WarnEx(NULL, "This channel is already in use, continuing anyway.  Use GPIO.setwarnings(False) to disable warnings.", 1);
   }
#else
#endif

   if (direction == OUTPUT && (initial == LOW || initial == HIGH))
   {
      output_gpio(gpio, initial);
   }
   setup_gpio(gpio, direction, pud);
   gpio_direction[gpio] = direction;

   Py_RETURN_NONE;
}

// python function output(channel, value)
static PyObject *py_output_gpio(PyObject *self, PyObject *args)
{
   unsigned int gpio;
   int channel, value;

   if (!PyArg_ParseTuple(args, "ii", &channel, &value))
      return NULL;

   if (get_gpio_number(channel, &gpio))
       return NULL;

   if (gpio_direction[gpio] != OUTPUT)
   {
      PyErr_SetString(PyExc_RuntimeError, "The GPIO channel has not been set up as an OUTPUT");
      return NULL;
   }

   if (check_gpio_priv())
      return NULL;

   output_gpio(gpio, value);
   Py_RETURN_NONE;
}

// python function value = input(channel)
static PyObject *py_input_gpio(PyObject *self, PyObject *args)
{
   unsigned int gpio;
   int channel;
   PyObject *value;

   if (!PyArg_ParseTuple(args, "i", &channel))
      return NULL;

   if (get_gpio_number(channel, &gpio))
       return NULL;

   // check channel is set up as an input or output
   if (gpio_direction[gpio] != INPUT && gpio_direction[gpio] != OUTPUT)
   {
      PyErr_SetString(PyExc_RuntimeError, "You must setup() the GPIO channel first");
      return NULL;
   }

   if (check_gpio_priv())
      return NULL;

   if (input_gpio(gpio)) {
      value = Py_BuildValue("i", HIGH);
   } else {
      value = Py_BuildValue("i", LOW);
   }
   return value;
}

// python function setmode(mode)
static PyObject *py_setmode(PyObject *self, PyObject *args)
{
   if (!PyArg_ParseTuple(args, "i", &gpio_mode))
      return NULL;

   if (setup_error)
   {
      PyErr_SetString(PyExc_RuntimeError, "Module not imported correctly!");
      return NULL;
   }

   if (gpio_mode != BOARD && gpio_mode != BCM && gpio_mode != MODE_RAW)
   {
      PyErr_SetString(PyExc_ValueError, "An invalid mode was passed to setmode()");
      return NULL;
   }

   Py_RETURN_NONE;
}

static unsigned int chan_from_gpio(unsigned int gpio)
{
   int chan;

   if (gpio_mode == BCM)
      return gpio;
   if (gpio_mode == MODE_RAW)
       return -1;  //Not supported in RAW mode as there is not always a mapping
   for (chan=1; chan<65; chan++)
      if (*(*pin_to_gpio+chan) == gpio)
         return chan;
   return -1;
}

static void run_py_callbacks(unsigned int gpio)
{
   PyObject *result;
   PyGILState_STATE gstate;
   struct py_callback *cb = py_callbacks;

   while (cb != NULL)
   {
      if (cb->gpio == gpio) {
         // run callback
         gstate = PyGILState_Ensure();
         result = PyObject_CallFunction(cb->py_cb, "i", chan_from_gpio(gpio));
         if (result == NULL && PyErr_Occurred()){
            PyErr_Print();
            PyErr_Clear();
         }
         Py_XDECREF(result);
         PyGILState_Release(gstate);
      }
      cb = cb->next;
   }
}

static int add_py_callback(unsigned int gpio, PyObject *cb_func)
{
   struct py_callback *new_py_cb;
   struct py_callback *cb = py_callbacks;	
	
   // add callback to py_callbacks list
   new_py_cb = malloc(sizeof(struct py_callback));
   if (new_py_cb == 0)
   {
      PyErr_NoMemory();
      return -1;
   }
   new_py_cb->py_cb = cb_func;
   Py_XINCREF(cb_func);         // Add a reference to new callback
   new_py_cb->gpio = gpio;
   new_py_cb->next = NULL;
   if (py_callbacks == NULL) {
      py_callbacks = new_py_cb;
   } else {
      // add to end of list
      while (cb->next != NULL)
         cb = cb->next;
      cb->next = new_py_cb;
   }
   add_edge_callback(gpio, run_py_callbacks);
   return 0;
}

// python function add_event_callback(gpio, callback)
static PyObject *py_add_event_callback(PyObject *self, PyObject *args, PyObject *kwargs)
{
   unsigned int gpio;
   int channel;
   PyObject *cb_func;
   char *kwlist[] = {"gpio", "callback", NULL};

   if (!PyArg_ParseTupleAndKeywords(args, kwargs, "iO|i", kwlist, &channel, &cb_func))
      return NULL;

   if (!PyCallable_Check(cb_func))
   {
      PyErr_SetString(PyExc_TypeError, "Parameter must be callable");
      return NULL;
   }

   if (get_gpio_number(channel, &gpio))
       return NULL;
       
   // check channel is set up as an input
   if (gpio_direction[gpio] != INPUT)
   {
      PyErr_SetString(PyExc_RuntimeError, "You must setup() the GPIO channel as an input first");
      return NULL;
   }

   if (!gpio_event_added(gpio))
   {
      PyErr_SetString(PyExc_RuntimeError, "Add event detection using add_event_detect first before adding a callback");
      return NULL;
   }

   if (add_py_callback(gpio, cb_func) != 0)
      return NULL;

   Py_RETURN_NONE;
}

// python function add_event_detect(gpio, edge, callback=None, bouncetime=0
static PyObject *py_add_event_detect(PyObject *self, PyObject *args, PyObject *kwargs)
{
   unsigned int gpio;
   int channel, edge, result;
   unsigned int bouncetime = 0;
   PyObject *cb_func = NULL;
   char *kwlist[] = {"gpio", "edge", "callback", "bouncetime", NULL};

   if (!PyArg_ParseTupleAndKeywords(args, kwargs, "ii|Oi", kwlist, &channel, &edge, &cb_func, &bouncetime))
      return NULL;

   if (cb_func != NULL && !PyCallable_Check(cb_func))
   {
      PyErr_SetString(PyExc_TypeError, "Parameter must be callable");
      return NULL;
   }

   if (get_gpio_number(channel, &gpio))
       return NULL;

   // check channel is set up as an input
   if (gpio_direction[gpio] != INPUT)
   {
      PyErr_SetString(PyExc_RuntimeError, "You must setup() the GPIO channel as an input first");
      return NULL;
   }

   // is edge valid value
   edge -= PY_EVENT_CONST_OFFSET;
   if (edge != RISING_EDGE && edge != FALLING_EDGE && edge != BOTH_EDGE)
   {
      PyErr_SetString(PyExc_ValueError, "The edge must be set to RISING, FALLING or BOTH");
      return NULL;
   }

   if (check_gpio_priv())
      return NULL;
	
   if ((result = add_edge_detect(gpio, edge, bouncetime)) != 0)   // starts a thread
   {
      if (result == 1)
      {
         PyErr_SetString(PyExc_RuntimeError, "Edge detection already enabled for this GPIO channel");
         return NULL;
      } else {
         PyErr_SetString(PyExc_RuntimeError, "Failed to add edge detection");
         return NULL;
      }
   }

   if (cb_func != NULL)
      if (add_py_callback(gpio, cb_func) != 0)
         return NULL;

   Py_RETURN_NONE;
}

// python function remove_event_detect(gpio)
static PyObject *py_remove_event_detect(PyObject *self, PyObject *args)
{
   unsigned int gpio;
   int channel;
   struct py_callback *cb = py_callbacks;
   struct py_callback *temp;
   struct py_callback *prev = NULL;

   if (!PyArg_ParseTuple(args, "i", &channel))
      return NULL;

   if (get_gpio_number(channel, &gpio))
       return NULL;

   // remove all python callbacks for gpio
   while (cb != NULL)
   {
      if (cb->gpio == gpio)
      {
         Py_XDECREF(cb->py_cb);
         if (prev == NULL)
            py_callbacks = cb->next;
         else
            prev->next = cb->next;
         temp = cb;
         cb = cb->next;
         free(temp);
      } else {
         prev = cb;
         cb = cb->next;
      }
   }

   if (check_gpio_priv())
      return NULL;

   remove_edge_detect(gpio);

   Py_RETURN_NONE;
}

// python function value = event_detected(channel)
static PyObject *py_event_detected(PyObject *self, PyObject *args)
{
   unsigned int gpio;
   int channel;

   if (!PyArg_ParseTuple(args, "i", &channel))
      return NULL;

   if (get_gpio_number(channel, &gpio))
       return NULL;

   if (event_detected(gpio))
      Py_RETURN_TRUE;
   else
      Py_RETURN_FALSE;
}

// python function py_wait_for_edge(gpio, edge)
static PyObject *py_wait_for_edge(PyObject *self, PyObject *args)
{
   unsigned int gpio;
   int channel, edge, result;
   char error[30];

   if (!PyArg_ParseTuple(args, "ii", &channel, &edge))
      return NULL;

   if (get_gpio_number(channel, &gpio))
      return NULL;

   // check channel is setup as an input
   if (gpio_direction[gpio] != INPUT)
   {
      PyErr_SetString(PyExc_RuntimeError, "You must setup() the GPIO channel as an input first");
      return NULL;
   }

   // is edge a valid value?
   edge -= PY_EVENT_CONST_OFFSET;
   if (edge != RISING_EDGE && edge != FALLING_EDGE && edge != BOTH_EDGE)
   {
      PyErr_SetString(PyExc_ValueError, "The edge must be set to RISING, FALLING or BOTH");
      return NULL;
   }

   if (check_gpio_priv())
      return NULL;

   Py_BEGIN_ALLOW_THREADS // disable GIL
   result = blocking_wait_for_edge(gpio, edge);
   Py_END_ALLOW_THREADS   // enable GIL

   if (result == 0) {
      Py_INCREF(Py_None);
      return Py_None;
   } else if (result == 1) {
      PyErr_SetString(PyExc_RuntimeError, "Edge detection events already enabled for this GPIO channel");
      return NULL;
   } else {
      sprintf(error, "Error #%d waiting for edge", result);
      PyErr_SetString(PyExc_RuntimeError, error);
      return NULL;
   }

   Py_RETURN_NONE;
}

// python function value = gpio_function(channel)
static PyObject *py_gpio_function(PyObject *self, PyObject *args)
{
   unsigned int gpio;
   int channel;
   int f;
   PyObject *func;

   if (!PyArg_ParseTuple(args, "i", &channel))
      return NULL;

   if (get_gpio_number(channel, &gpio))
       return NULL;

   if (mmap_gpio_mem())
      return NULL;

   f = gpio_function(gpio);
//printf("### %s: f=%d, gpio=%d ###\n",__func__, f, gpio);
     switch (f)
	{
		case 0 : f = INPUT; break;
		case 1 : f = OUTPUT; break;
		// ALT 0
		case 4 : switch (gpio)
				{
					case 257 :
					case 256 :
					case 53 :
					case 52 : f = I2C; break;
					case 270 :
					case 266 :
					case 269 :
					case 268 :
					case 267 : f = SPI; break;
					case 276 :
					case 45 : f = PWM; break;
					case 228 :
					case 229 : f = SERIAL; break;
					//case 28 :
					//case 29 : f = I2C; break;
					default : f = MODE_UNKNOWN; break;
				}
				break;
	// ALT 5
	case 2 : if (gpio == 259 || gpio == 39) f = PWM; else f = MODE_UNKNOWN;
			break;
	// ALT 4	
	case 3 : switch (gpio)
			{
				case 38 :
				case 275 :
				case 259 :
				case 39 :
				case 44 :
				case 40 : f = SPI; break;
				default : f = MODE_UNKNOWN; break;
			}
			break;
	default : f = MODE_UNKNOWN; break;
	} 
	
   func = Py_BuildValue("i", f);
   return func;
}

// python function setwarnings(state)
static PyObject *py_setwarnings(PyObject *self, PyObject *args)
{
   if (!PyArg_ParseTuple(args, "i", &gpio_warnings))
      return NULL;

   if (setup_error)
   {
      PyErr_SetString(PyExc_RuntimeError, "Module not imported correctly!");
      return NULL;
   }

   Py_RETURN_NONE;
}

static const char moduledocstring[] = "GPIO functionality of a Banana Pi using Python";

PyMethodDef rpi_gpio_methods[] = {
   {"setup", (PyCFunction)py_setup_channel, METH_VARARGS | METH_KEYWORDS, "Set up the GPIO channel, direction and (optional) pull/up down control\nchannel        - either board pin number or BCM number depending on which mode is set.\ndirection      - INPUT or OUTPUT\n[pull_up_down] - PUD_OFF (default), PUD_UP or PUD_DOWN\n[initial]      - Initial value for an output channel"},
   {"cleanup", (PyCFunction)py_cleanup, METH_VARARGS | METH_KEYWORDS, "Clean up by resetting all GPIO channels that have been used by this program to INPUT with no pullup/pulldown and no event detection\n[channel] - individual channel to clean up.  Default - clean every channel that has been used."},
   {"output", py_output_gpio, METH_VARARGS, "Output to a GPIO channel\nchannel - either board pin number or BCM number depending on which mode is set.\nvalue   - 0/1 or False/True or LOW/HIGH"},
   {"input", py_input_gpio, METH_VARARGS, "Input from a GPIO channel.  Returns HIGH=1=True or LOW=0=False\nchannel - either board pin number or BCM number depending on which mode is set."},
   {"setmode", py_setmode, METH_VARARGS, "Set up numbering mode to use for channels.\nBOARD - Use Raspberry Pi board numbers\nBCM   - Use Broadcom GPIO 00..nn numbers"},
   {"add_event_detect", (PyCFunction)py_add_event_detect, METH_VARARGS | METH_KEYWORDS, "Enable edge detection events for a particular GPIO channel.\nchannel      - either board pin number or BCM number depending on which mode is set.\nedge         - RISING, FALLING or BOTH\n[callback]   - A callback function for the event (optional)\n[bouncetime] - Switch bounce timeout in ms for callback"},
   {"remove_event_detect", py_remove_event_detect, METH_VARARGS, "Remove edge detection for a particular GPIO channel\nchannel - either board pin number or BCM number depending on which mode is set."},
   {"event_detected", py_event_detected, METH_VARARGS, "Returns True if an edge has occured on a given GPIO.  You need to enable edge detection using add_event_detect() first.\nchannel - either board pin number or BCM number depending on which mode is set."},
   {"add_event_callback", (PyCFunction)py_add_event_callback, METH_VARARGS | METH_KEYWORDS, "Add a callback for an event already defined using add_event_detect()\nchannel      - either board pin number or BCM number depending on which mode is set.\ncallback     - a callback function"},
   {"wait_for_edge", py_wait_for_edge, METH_VARARGS, "Wait for an edge.\nchannel - either board pin number or BCM number depending on which mode is set.\nedge    - RISING, FALLING or BOTH"},
   {"gpio_function", py_gpio_function, METH_VARARGS, "Return the current GPIO function (IN, OUT, PWM, SERIAL, I2C, SPI)\nchannel - either board pin number or BCM number depending on which mode is set."},
   {"setwarnings", py_setwarnings, METH_VARARGS, "Enable or disable warning messages"},
   {NULL, NULL, 0, NULL}
};

#if PY_MAJOR_VERSION > 2
static struct PyModuleDef rpigpiomodule = {
   PyModuleDef_HEAD_INIT,
   "RPi.GPIO",       // name of module
   moduledocstring,  // module documentation, may be NULL
   -1,               // size of per-interpreter state of the module, or -1 if the module keeps state in global variables.
   rpi_gpio_methods
};
#endif

#if PY_MAJOR_VERSION > 2
PyMODINIT_FUNC PyInit_GPIO(void)
#else
PyMODINIT_FUNC initGPIO(void)
#endif
{
   int i;
   PyObject *module = NULL;

#if PY_MAJOR_VERSION > 2
   if ((module = PyModule_Create(&rpigpiomodule)) == NULL)
      return NULL;
#else
   if ((module = Py_InitModule3("RPi.GPIO", rpi_gpio_methods, moduledocstring)) == NULL)
      return;
#endif

   define_constants(module);

   for (i=0; i<64; i++)
      gpio_direction[i] = -1;

   // detect board revision and set up accordingly
   revision = get_rpi_revision();
D	printf("BAPI: revision(%d)\n",revision);
   if (revision == -1)
   {
      PyErr_SetString(PyExc_RuntimeError, "This module can only be run on a Raspberry Pi!");
      setup_error = 1;
#if PY_MAJOR_VERSION > 2
      return NULL;
#else
      return;
#endif
   } else if (revision == 1) {
      pin_to_gpio = &physToGpio_BP;//&pin_to_gpio_rev1;
   } else { // assume revision 2
      //pin_to_gpio = &pin_to_gpio_rev2;
      pin_to_gpio = NULL;			//here is the 'pin_to_gpio' initialization
   }

   rpi_revision = Py_BuildValue("i", revision);
   PyModule_AddObject(module, "RPI_REVISION", rpi_revision);

   // Add PWM class
   if (PWM_init_PWMType() == NULL)
#if PY_MAJOR_VERSION > 2
      return NULL;
#else
      return;
#endif
   Py_INCREF(&PWMType);
   PyModule_AddObject(module, "PWM", (PyObject*)&PWMType);

   if (!PyEval_ThreadsInitialized())
      PyEval_InitThreads();

   // register exit functions - last declared is called first
   if (Py_AtExit(cleanup) != 0)
   {
      setup_error = 1;
      cleanup();
#if PY_MAJOR_VERSION > 2
      return NULL;
#else
      return;
#endif
   }

   if (Py_AtExit(event_cleanup_all) != 0)
   {
      setup_error = 1;
      cleanup();
#if PY_MAJOR_VERSION > 2
      return NULL;
#else
      return;
#endif
   }

#if PY_MAJOR_VERSION > 2
   return module;
#else
   return;
#endif
}
