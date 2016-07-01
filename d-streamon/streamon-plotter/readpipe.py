import os
import time
import errno

fifoname = "../../pprova"
if not os.path.exists(fifoname):
	os.mkfifo(fifoname, 0777)

#pipe = open(fifoname, os.O_RDONLY | os.O_NONBLOCK)
line = None
pipe = os.open(fifoname, os.O_RDONLY)
pipefile = os.fdopen(pipe)
print "pipe opened"

while(1):
    buffer = os.read(pipe, 1024)
	#line = pipe.readline(  )[:-1]
    if buffer:
        print 'got "%s" at %s' % (buffer, time.time())

print "exit"



'''try:
    #line = pipefile.readline(  )[:-1]
    buffer = os.read(pipe, 1024)

except OSError as err:
    if err.errno == errno.EAGAIN or err.errno == errno.EWOULDBLOCK:
	print "EAGAIN"
        buffer = None
    else:
        raise  # something else has happened -- better reraise

if line is None:
    print "nothing received"
    # nothing was received -- do something else
else:
    print 'got "%s" at %s' % (line, time.time())
    # buffer contains some received data -- do something with it'''
