#!/usr/bin/python
import atexit
import readIPC
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
start_time = time.time()

#These callback allow the graph to update its x axis range
#automatically when the mouse is
#outside the window
def on_mouse_enter(event):
	global entered
	entered = True
	
def on_mouse_leave(event):
	global entered
	entered = False

f1, f2, f3, f4 = None, None, None, None

if len(sys.argv) < 4:
    print "syntax: streamon_plotter <pipename> <key> <feature1> [feature2] [feature3] [feature4]"
    exit()


name = sys.argv[1]
key = sys.argv[2]

fig = plt.figure()
plt.title("Streamon plotter")
plt.xlabel('Timestamp')
ax = fig.add_subplot(111)
xdata = []

nfeatures = len(sys.argv) - 3

if nfeatures >= 1:
    f1 = True
    f1Data = []
    f1Name = sys.argv[3]
    f1Line, = ax.plot([], [], animated=True, lw=2, label=f1Name)

if nfeatures >= 2:
    f2 = True
    f2Data = []
    f2Name = sys.argv[4]
    f2Line, = ax.plot([], [], animated=True, lw=2, label=f2Name)

if nfeatures >= 3:
    f3 = True
    f3Data = []
    f3Name = sys.argv[5]
    f3Line, = ax.plot([], [], animated=True, lw=2, label=f3Name)

if nfeatures >= 4:
    f4 = True
    f4Data = []
    f4Name = sys.argv[6]
    f4Line, = ax.plot([], [], animated=True, lw=2, label=f4Name)

fig.canvas.mpl_connect('figure_enter_event', on_mouse_enter)
fig.canvas.mpl_connect('figure_leave_event', on_mouse_leave)

ax.set_ylim(0, 30)
ax.set_xlim(0, 10)
canvas = ax.figure.canvas
background = canvas.copy_from_bbox(ax.bbox)
ax.legend(bbox_to_anchor=(0., 1.02, 1., .102), loc=1, ncol=6, mode="expand", borderaxespad=0.)

uom, label = '', ''
nbCrit, nbWarn = 0,0

fifoname = "../%s"%(name)
if not os.path.exists(fifoname):
	os.mkfifo(fifoname, 0777)

pipe = os.open(fifoname, os.O_RDONLY | os.O_NONBLOCK)
pipefile = os.fdopen(pipe)

#callback function for updating plot
def update():
    global entered, uom, label, nbCrit, nbWarn
    f1Draw, f2Draw, f3Draw, f4Draw = None, None, None, None
    canvas.restore_region(background)
    last_x = 0
    last_y_values = [0]

    try:
        buffer = os.read(pipe, 512)
        if buffer is None:
            return True
        # nothing was received -- do something else
        else:
            print 'got "%s" at %s' % (buffer, time.time())
        # buffer contains some received data -- do something with it
    except OSError as err:
        if err.errno == errno.EAGAIN or err.errno == errno.EWOULDBLOCK:
            return True

    try:
        jmsg = json.loads(buffer)

    except:
        print "exception"
        return True

    print "encoded son msg"
    print jmsg

    stream = jmsg["stream"]
    jkey = jmsg["key"]
    if key != "any" and jkey != key:
        return True

    elements = jmsg["elements"]
    n_elements = len(elements)

    print "elements"
    print elements

    now = time.time() - start_time
    xdata.append(now)
    xmin, xmax = ax.get_xlim()
    last_x = xdata[-1]
    ymin, ymax = ax.get_ylim()
    
    plt.ylabel("feature values")
 
    xmed = (xmax - xmin / 2)
    y_ref = (ymax - ymin) * 3/4

    try:
        for i in range(0, n_elements):    
            last_y_values.append(elements[i]["value"])
            if f1 and elements[i]["name"] == f1Name:
                f1Data.append(elements[i]["value"])
                f1Line.set_data(xdata, f1Data)
                f1Draw = True
            if f2 and elements[i]["name"] == f2Name:
                f2Data.append(elements[i]["value"])
                f2Line.set_data(xdata, f2Data)
                f2Draw = True
            if f3 and elements[i]["name"] == f3Name:
                f3Data.append(elements[i]["value"])
                f3Line.set_data(xdata, f3Data)
                f3Draw = True
            if f4 and elements[i]["name"] == f4Name:
                f4Data.append(elements[i]["value"])
                f4Line.set_data(xdata, f4Data)
                f4Draw = True 
    except AssertionError:
        pass
        return True
 

    if (last_x > xmed) and not entered:
        ax.set_xlim(last_x-20, last_x + 20)
        fig.canvas.draw()
 
    if (max(last_y_values) > y_ref) and not entered:
        ax.set_ylim(ymin, ymax + y_ref / 2)
        fig.canvas.draw()


    if f1Draw:
        ax.draw_artist(f1Line)
    if f2Draw:
        ax.draw_artist(f2Line)
    if f3Draw:
        ax.draw_artist(f3Line)
    if f4Draw:
        ax.draw_artist(f4Line)

    try:
        canvas.blit(ax.bbox)
    except AttributeError:
        pass
        return True 

    return True 


gobject.timeout_add(500, update)
plt.show()

