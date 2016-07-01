#!/usr/bin/python
import readIPC#, sysv_ipc
import re
import sys
import random
import math
import time
import json
import os
import errno

import gobject
from matplotlib.widgets import Cursor
import matplotlib
matplotlib.use('GTKAgg')
import matplotlib.pyplot as plt

entered = False

#These callback allow the graph to update its x axis range
#automatically when the mouse is
#outside the window
def on_mouse_enter(event):
	global entered
	entered = True
	
def on_mouse_leave(event):
	global entered
	entered = False


#initialize plot
print "len :%d"%(len(sys.argv)) 

if len(sys.argv) < 2:
    print "syntax: streamon_plotter <key_to_plot> <feature1> <feature2> <feature3> <...>"
    exit()

key = sys.argv[1]
fig = plt.figure()



plt.title("Streamon feature graph for key %s"%(key))
plt.xlabel('Timestamp')
ax = fig.add_subplot(111)
xdata, ydata, ymax, ywarn, ymin, ycrit = [], [], [], [], [], []

fig.canvas.mpl_connect('figure_enter_event', on_mouse_enter)
fig.canvas.mpl_connect('figure_leave_event', on_mouse_leave)

valueLine, = ax.plot([], [], animated=True, lw=1, label="value")
maxLine, = ax.plot([], [], animated=True, lw=1, color='grey', label="max")
#minLine, = ax.plot([], [], animated=True, lw=1, color='green', label="min")
#warnLine, = ax.plot([], [], animated=True, lw=1, color='orange', label="warn")
#critLine, = ax.plot([], [], animated=True, lw=1, color='red', label="crit")

ax.set_ylim(0, 40)
ax.set_xlim(0, 10)
canvas = ax.figure.canvas
background = canvas.copy_from_bbox(ax.bbox)
ax.legend(bbox_to_anchor=(0., 1.02, 1., .102), loc=1, ncol=6, mode="expand", borderaxespad=0.)

uom, label = '', ''
nbCrit, nbWarn = 0,0

fifoname = "../p_scan"
if not os.path.exists(fifoname):
	os.mkfifo(fifoname, 0777)

line = None
pipe = os.open(fifoname, os.O_RDONLY | os.O_NONBLOCK)
pipefile = os.fdopen(pipe)
print "pipe opened"

#callback function for updating plot
def update():
    global entered, uom, label, nbCrit, nbWarn
    canvas.restore_region(background)
    last_x, first_x = 0, 0

    try:
        buffer = os.read(pipe, 1024)
        if buffer is None or buffer == "":
            print "nothing received"
            return True
        # nothing was received -- do something else
        else:
            print 'got "%s" at %s' % (buffer, time.time())
        # buffer contains some received data -- do something with it
    except OSError as err:
        if err.errno == errno.EAGAIN or err.errno == errno.EWOULDBLOCK:
            print "EAGAIN"
            return True

    try:
        jmsg = json.loads(buffer)

    except:
        print "exception"
        return True

    print "encoded son msg"
    print jmsg

    stream = jmsg["stream"]
    key = jmsg["key"]
    elements = jmsg["elements"]
    n_elements = len(elements)

    now = time.time()
    rand = random.random()
    #value = round(2+(rand/2), 3)
    value = elements[0]["value"]
    points = [[now, value]]
    maximum = 30
    minimum = 0
    warn = 200
    crit = 200
    label = ""
    uom = "ms"
    print points

    for point in points:
        xdata.append(point[0])
        ydata.append(point[1])
        ymax.append(maximum)
        #ymin.append(minimum)
        #ywarn.append(warn)
        #ycrit.append(crit)
        print int(crit), point[1], nbCrit

        if int(crit) <= point[1]:
            nbCrit+=1
        elif int(warn)<= point[1]:
            nbWarn+=1
	
    xmin, xmax = ax.get_xlim()
    if uom == '' : uom = 'no unit' 
    plt.ylabel( label + ' (' + uom + ')')
    last_x = xdata[-1]
    first_x = xdata[0]
    #update the x axis display 

    if (last_x > xmax) and not entered:
        ax.set_xlim(last_x, last_x + 50)
        fig.canvas.draw()
		
    if last_x - first_x > 50 and not entered:
        ax.set_xlim(last_x-50, last_x + 10)
        fig.canvas.draw()

    valueLine.set_data(xdata, ydata)
    maxLine.set_data(xdata, ymax)
    #warnLine.set_data(xdata, ywarn)
    #critLine.set_data(xdata, ycrit)
    #minLine.set_data(xdata, ymin)

    try:
        #Draw the lines
        ax.draw_artist(valueLine)
        ax.draw_artist(maxLine)
        #ax.draw_artist(minLine)
        #ax.draw_artist(critLine)
        #ax.draw_artist(warnLine)
    except AssertionError:
        pass
    try:
        canvas.blit(ax.bbox)
    except AttributeError:
        pass
    return True 

def savegraph():
    return

import atexit
atexit.register(savegraph)


gobject.timeout_add(500, update)
plt.show()
