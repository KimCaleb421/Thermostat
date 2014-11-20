#! /usr/bin/env python

import sys, serial,time, datetime

filename = time.strftime("%m\%d_%H:%M:%S")
dataFile = open("./%s.csv" % filename, "w");

