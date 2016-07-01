#!/usr/bin/python

import sysv_ipc
import random
import math
import time

mq = sysv_ipc.MessageQueue(key=0x0001e240, flags=sysv_ipc.IPC_CREAT)

i=0
while 1:
	now = time.time()
	rand = random.gauss(5, 1)
	value = round(rand, 3)
	perfdata = "time=" + str(value) + "ms;4;7;0;10"
	message = str(int(now)) + "^" + perfdata
	print "Send", message, "to ipc queue ..."
	mq.send(message)
	i+=0.01
	if i > 1:
		i=0
	time.sleep(1)
