from twisted.internet import reactor
from txjsonrpc.web.jsonrpc import Proxy
import simplejson
from mytest import MyTest

def printValue(value):
    data = simplejson.loads(value)
    m = MyTest()
    m.__dict__ = data
    print "foo=", m.get_foo()
    print "bar=", m.get_bar()

def printError(error):
    print 'error', error

def shutDown(data):
    print "Shutting down reactor..."
    reactor.stop()

proxy = Proxy('http://127.0.0.1:7080/')

d = proxy.callRemote('add', 3, 5)
d.addCallback(printValue).addErrback(printError).addBoth(shutDown)
reactor.run()
