import os
import time


i = 0
fifoname = "../../pprova"
pipe = os.open(fifoname, os.O_WRONLY)

print "opened"
os.write(pipe, "ciao")	

while(True):
    print "."
    i += 1
    time.sleep(5)
    os.write(pipe, "%s"%(i))

print "ciao"


