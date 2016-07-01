#!/usr/bin/python
#import sysv_ipc
import re
import sys

#mq = sysv_ipc.MessageQueue(key=0x0001e240)

def get_cuurent_data():
	"""u This function consumes all the messages that are present in the 
	IPC queue at the moment of call. If there is no message at all ,
	it raises a	BusyError exception.
	Return a list of points where each point is a list of two elements.
	The first element is the x coordinate which is the timstamp of the message.
	The second element is the y coordinate which is the value of perfdata.
	"""
	global mq
	points = []
	while 1:
		#try:
			#non blocking message read to avoid hangs in interface
		#	message = mq.receive(False)[0]
		#except sysv_ipc.BusyError:
		#	if not points:
		#		raise #no message in the queue
		#	else:
		#		return points, uom, label, minimum, maximum, warn, crit
		message="1368538088^time=2.253ms;4;7;0;10"
		timestamp, perfdata = message.split("^")
		#extraction des informations du pefdata nagios
		labelValueUom,warn,crit,minimum,maximum = perfdata.split(";")
		label, equalSign, valueUom = labelValueUom.rpartition("=")
		p = re.compile('([-+]?\d*\.\d+|\d+)(.*)')
		m = p.match(valueUom)
		value, uom = m.group(1, 2)
		points.append([int(timestamp),float(value)])
