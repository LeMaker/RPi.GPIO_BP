#!/bin/sh
rm -rf /usr/local/lib/python2.7/dist-packages/*
python setup.py install
cd myTest
python led.py
