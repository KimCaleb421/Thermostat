# plotting.py
# Tim Kerins 21-04-14
# A very simple datalogger script that parses data from the serial port and
# saves it in the file loggedData.dat

#! /usr/bin/env python

import sys, serial,time, datetime
from matplotlib import pyplot as plt

# set the serial port the Arduino is connected to
serPort = '/dev/ttyACM0'

# open the file for writing
filename = time.strftime("%m\%d_%H:%M:%S")
dataFile = open("./%s.csv" % filename, "w");


print "\n********************************************************"
print "\nLog Data"
print "\nAttempting to open Serial Port : ",serPort,"for logging\n"

# opens usb serial port for loggig
ser = serial.Serial(serPort,9600,timeout=1)

# checks the port is open 
if (ser.isOpen() == False):
    print "ERROR : Unable to open serial port ",serPort,"\n"
    exit(0);
else:
    print "Port ",serPort," opened\n"

# force print to console
sys.stdout.flush()

# waits for signal from user to start logging
print "Hit return to start logging ..."
key = sys.stdin.readline()
start = time.time()

# sends signal to start logging
ser.write('1');
print "Logging Started. CTRL-C to stop\n"

#def trunc(f,n):
#	slen = len('%.*f' % (n,f))
#	return str(f)[:slen]

while True:
    try:

        # read data from serial writes to stdio and dataFile
        line = ser.readline()
	elapsed = time.time() - start
	print str("{:.2f}".format(elapsed)) + ',',
	#print str(','),       
# print str(datetime.timedelta(seconds=elapsed)),  #trunc(elapsed,2))),
	print line;
        dataFile.write(line)
        
    except KeyboardInterrupt: #CTRL-C triggered here 

        # sends signal to stop logging    
        ser.write('0')

        print "Logging Stopped\n"

        break;

# close the serial port
ser.flush()
ser.close()

# close the datafile
dataFile.close()

print "Port ",serPort," closed\n"
print "\n********************************************************\n"

#print "\nPlot graph (y/n) ?"
#key = sys.stdin.read(1)
#if key=='y':

 #   print "\nPlotting graph ...\n"

    # now plot the graph

    # read all of the data from the textfile
  #  f = open("loggedData.dat",'r')
   # lines = f.readlines()
   # f.close()

    # intialize variables to be lists
 #   x = []
  #  y = []

    # lines and add to lists
   # for line in lines:
    #    p=line.split(',')
     #   x.append(float(p[0]))
      #  y.append(float(p[1]))
    
   # fig=plt.figure()
   # graph=fig.add_subplot(111)
   # graph.set_title("Data logged from Arduino UNO")
   # graph.plot(x,y,'ro')
   # plt.show()

   # print "Plot complete\n"   

#else:

   # print "Finishing\n"
# rest for 3 seconds
time.sleep(3)
