from twisted.internet import reactor
from txjsonrpc.web.jsonrpc import Proxy
import jsonpickle
import sys
import os
sys.path.append("/home/fhuici/research/eu/demons/svn/Sources/blockmon/main/node/daemon/")
from core.returnvalue import *
from core.host import HostSpecsInfo
from core.block import *

DAEMON_PORT = 7080

def print_results(value):
    r = jsonpickle.decode(value)
    print str(r)

def print_readvar_results(value):
    r = jsonpickle.decode(value)
    print str(r)
    for v in r.get_value():
        print str(v)

def print_block_infos(value):
    r = jsonpickle.decode(value)
    for block in r.get_value():
        print str(block)

def print_error(error):
    print 'error', error

def shut_down(data):
    print "Shutting down reactor..."
    reactor.stop()

def shut_down_harsh(data):
    print "Shutting down reactor..."
    os._exit(1)



bm_daemon = Proxy('http://127.0.0.1:' + str(DAEMON_PORT) + '/')

value = int(sys.argv[1])

if value == 1:
    d = bm_daemon.callRemote('get_blocks_list')
    d.addCallback(print_results).addErrback(print_error).addBoth(shut_down)

elif value == 2:    
    d = bm_daemon.callRemote('get_hw_specs')
    d.addCallback(print_results).addErrback(print_error).addBoth(shut_down)

elif value == 3:    
    b = ['SketchMerger', 'L4Demux', 'PcapSource', 'ComboSZE2Source', 'PerFlowStats', 'PktPairifier', 'Null', 'SynthSource', 'CDFGenerator', 'SynFloodDetector', 'SketchFlowCounter', 'PacketPrinter', 'TCPFlagCounter', 'PktCounter', 'IPFIXExporter', 'RRDemux', 'TopNFlowSelector', 'PFQSource', 'IpDumbAnonymizer', 'IPFIXSource', 'PFRingSource']
    d = bm_daemon.callRemote('get_blocks_info', b)
    d.addCallback(print_block_infos).addErrback(print_error).addBoth(shut_down)

elif value == 4:    
    f = open("/home/fhuici/research/eu/demons/svn/Sources/blockmon/main/node/daemon/tests/compositions/snifferctr.xml", "r")
    c = f.read()
    f.close()
    d = bm_daemon.callRemote('start_composition', c)
    d.addCallback(print_results).addErrback(print_error).addBoth(shut_down)

elif value == 5:    
    d = bm_daemon.callRemote('stop_composition', 'mysnifferctr')
    d.addCallback(print_results).addErrback(print_error).addBoth(shut_down)

elif value == 6:    
    d = bm_daemon.callRemote('get_composition_ids')
    d.addCallback(print_results).addErrback(print_error).addBoth(shut_down)

elif value == 7:    
    d = bm_daemon.callRemote('get_running_compositions', ['mysnifferctr'])
    d.addCallback(print_results).addErrback(print_error).addBoth(shut_down)

elif value == 8:    
    v1 = VariableInfo("counter", "pktcnt", "", "read")
    v2 = VariableInfo("counter", "bytecnt", "", "read")
    variables = jsonpickle.encode([v1, v2])
    d = bm_daemon.callRemote('read_variables', 'mysnifferctr', variables)
    d.addCallback(print_readvar_results).addErrback(print_error).addBoth(shut_down)

elif value == 9:
    f = open("/home/fhuici/research/eu/demons/svn/Sources/blockmon/main/node/daemon/tests/compositions/snifferctr_update.xml", "r")
    c = f.read()
    f.close()
    d = bm_daemon.callRemote('update_composition', c)
    d.addCallback(print_results).addErrback(print_error).addBoth(shut_down)

elif value == 10:
    variables = jsonpickle.encode([VariableInfo("counter", "reset", "", "write", "1")])
    d = bm_daemon.callRemote('write_variables', 'mysnifferctr', variables)
    d.addCallback(print_results).addErrback(print_error).addBoth(shut_down)    
reactor.run()
