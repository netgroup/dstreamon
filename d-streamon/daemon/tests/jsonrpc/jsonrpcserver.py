from txjsonrpc.web import jsonrpc
from twisted.web import server
from twisted.internet import reactor
from mytest import MyTest
import simplejson

class Math(jsonrpc.JSONRPC):
    """
    An example object to be published.
    """
    def jsonrpc_add(self, a, b):
        """
        Return sum of arguments.
        """
        m = MyTest()
        return simplejson.dumps(m.__dict__)

reactor.listenTCP(7080, server.Site(Math()))
reactor.run()
