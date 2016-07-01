#!/usr/bin/env python
from txjsonrpc.web import jsonrpc
from twisted.web import server
from twisted.internet import reactor
import xmlrpclib, pickle
import os
import sys
import ConfigParser
from core.returnvalue import *
from core.host import HostSpecsManager
from core.bmprocess import BMProcessInfo
from core.spawner import ProcessSpawner
from core.bmparser import CompositionParser
import jsonpickle
import logging
import commands
import random

class BMDaemon(jsonrpc.JSONRPC):

    """\brief Uses a json-rpc server on a well-known port to present a
              Blockmon API to the outside world (json is also used for 
              marshalling and demarshalling objects). All calls return a 
              core.returnvalue.ReturnValue object. Note that function
              documentation will list ReturnValue as the return type, but
              the human-readable description refers to ReturnValue.value.
              Run as root/sudo.

              The daemon works by spawning processes each running a separate
              instance of blockmon. In order to communicate with these, the
              BMDaemon uses xml-rpc and pickle. Note that core/bmprocess.py
              must be executable (u+x) in order for things to work.
    """

    def __init__(self, config):
        """\brief Initializes class
        \param config (\c string) The path to the daemon's configuration file
        """
        self.__bm_processes = {}    # [string]->BMProcessInfo (string: composition id)
        self.__config = config
        self.__parser = CompositionParser()
        self.__listening_port = None
        self.__bm_proc_exec = None
        self.__block_ext = None
        self.__blocks_path = None
        self.__logging_dir = None
        self.__bm_base_path = None
        self.__tmp_dir = None
        self.__logger = None
        self.__parse_config()
        self.__init_logging()
        self.__spawn_proc_id = 1
        self.__logger.info("starting bm daemon on port %s", str(self.__listening_port))

    def jsonrpc_get_blocks_list(self):
        """\brief Gets a list of the block types currently installed on the local host
        \return (\c ReturnValue) The block types (list[string])
        """
        f = None
        try:
            f = open(self.__bm_base_path + "/daemon/core/blockinfo.py")
        except IOError as e:
            msg = "No blockinfo.py file available, please run the " + \
                  "blockinfoparser.py script to generate this file"
            r = ReturnValue(ReturnValue.CODE_FAILURE, msg, None)
            return jsonpickle.encode(r)
        
        f.close()
        
        from core.blockinfo import block_infos
        r = ReturnValue(ReturnValue.CODE_SUCCESS, "", block_infos.keys())
        return jsonpickle.encode(r)

    def jsonrpc_get_hw_specs(self):
        """\brief Gets a description of the local host's hardware specs
        \return (\c ReturnValue) The specs (HostSpecsInfo)
        """
        mngr = HostSpecsManager()
        r = ReturnValue(ReturnValue.CODE_SUCCESS, "", mngr.get_host_specs())
        return jsonpickle.encode(r)

    def jsonrpc_get_blocks_info(self, block_types):
        """\brief Gets ful information about the given set of blocks
        \param  block_types (\c list[string]) The block types (e.g., ["PFQSource"])
        \return             (\c ReturnValue)  The information (list[BlockInfo])
        """
        f = None
        try:
            f = open(self.__bm_base_path + "/daemon/core/blockinfo.py")
        except IOError as e:
            msg = "No blockinfo.py file available, please run the " + \
                  "blockinfoparser.py script to generate this file"
            r = ReturnValue(ReturnValue.CODE_FAILURE, msg, None)
            return jsonpickle.encode(r)
        
        f.close()
        
        from core.blockinfo import block_infos
        blocks = []
        not_found = []
        for block_type in block_types:
            if block_infos.has_key(block_type):
                blocks.append(block_infos[block_type])
            else:
                not_found.append(block_type)

        msg = ""
        if len(not_found) > 0:
            msg = "not found:" + str(not_found)

        r = ReturnValue(ReturnValue.CODE_SUCCESS, msg, blocks)
        return jsonpickle.encode(r)

    def jsonrpc_start_composition(self, comp):
        """\brief Starts a new Blockmon instance based on the given 
                  composition XML. The instance will run in a newly
                  spawned process.
        \param  comp (\c string)      The composition XML
        \return      (\c ReturnValue) Value member is empty
        """
        bmproc_id = "bmprocess" + str(self.__spawn_proc_id)
        logfile = self.__logging_dir + "/" + bmproc_id + ".log"
        compfile = self.__tmp_dir + "/" + bmproc_id + ".xml"

        commands.getstatusoutput("mkdir -p " + self.__tmp_dir)
        f = open(compfile, "w")
        f.write(comp)
        f.close()
        
        port = str(self.__find_open_port())
        args = [compfile, logfile, port]
        proc = ProcessSpawner.spawn(self.__bm_proc_exec, args)        
        info = BMProcessInfo(proc, comp, logfile, port)
        self.__parser.set_comp(comp)
        comp_id = self.__parser.parse_comp_id()
        self.__bm_processes[comp_id] = info
        self.__spawn_proc_id += 1
        r = ReturnValue(ReturnValue.CODE_SUCCESS, "", None)

        self.__logger.info("spawned bm process on port " + port)
        print "spawned bm process on port " + port

        return jsonpickle.encode(r)

    def jsonrpc_update_composition(self, comp):
        """\brief Updates a Blockmon instance based on the given 
                  composition XML. 
        \param  comp (\c string)      The composition XML
        \return      (\c ReturnValue) Value member is empty
        """
        self.__parser.set_comp(comp)
        comp_id = self.__parser.parse_comp_id()
        port = self.__bm_processes[comp_id].get_port()
        url = "http://localhost:" + str(port) + "/"
        proxy = xmlrpclib.ServerProxy(url)
        r = pickle.loads(proxy.update_composition(comp))
        return jsonpickle.encode(r)        

    def jsonrpc_stop_composition(self, comp_id):
        """\brief Stops a Blockmon instance identified by the given 
                  composition id.
        \param  (\c string)      The composition's id
        \return (\c ReturnValue) Value member is empty
        """
        if not self.__bm_processes.has_key(comp_id):
            msg = "no composition with the given id exists, can't stop"
            ReturnValue(ReturnValue.CODE_SUCCESS, msg, None)
        port = self.__bm_processes[comp_id].get_port()
        url = "http://localhost:" + str(port) + "/"
        proxy = xmlrpclib.ServerProxy(url)
        r = pickle.loads(proxy.stop_composition())
        self.__bm_processes[comp_id].get_proc().kill()
        del self.__bm_processes[comp_id]
        self.__logger.info("killed bm process for composition " + comp_id)
        print "killed bm process for composition " + comp_id
        return jsonpickle.encode(r)
 
    def jsonrpc_get_composition_ids(self):
        """\brief Gets the ids of all compositions currently running
        \return (\c ReturnValue) The ids (list[string])
        """
        r = ReturnValue(ReturnValue.CODE_SUCCESS, "", self.__bm_processes.keys())
        return jsonpickle.encode(r)

    def jsonrpc_get_running_compositions(self, comp_ids):
        """\brief Gets the composition XML for the given ids
        \param comp_ids (\c list[string]) The ids
        \return         (\c ReturnValue)  The compositions' XML (list[string])
        """
        comps = []
        for comp_id in comp_ids:
            if self.__bm_processes.has_key(comp_id):
                comps.append(self.__bm_processes[comp_id].get_comp())
        r = ReturnValue(ReturnValue.CODE_SUCCESS, "", comps)
        return jsonpickle.encode(r)        

    def jsonrpc_read_variables(self, comp_id, json_variables):
        """\brief Reads a set of block variables. Each VariableInfo object
                  needs to have the name of the block and the variable name
                  set. The access_type member should be set to "read". The
                  function returns the same list, this time with the "value"
                  member filled out.
        \param comp_id        (\c string)             The composition id
        \param json_variables (\c list[VariableInfo]) Json-encode variables
        \return               (\c ReturnValue)        The values (list[VariableInfo])
        """
        if not self.__bm_processes.has_key(comp_id):
            msg = "no composition with the given id exists, can't read variables"
            ReturnValue(ReturnValue.CODE_SUCCESS, msg, None)
        variables = jsonpickle.decode(json_variables)
        port = self.__bm_processes[comp_id].get_port()
        url = "http://localhost:" + str(port) + "/"
        proxy = xmlrpclib.ServerProxy(url)
        pickled_variables = pickle.dumps(variables)
        r = pickle.loads(proxy.read_variables(pickled_variables))
        return jsonpickle.encode(r)

    def jsonrpc_write_variables(self, comp_id, json_variables):
        """\brief Writes to a set of block variables. Each VariableInfo object
                  needs to have the name of the block, the variable name and the
                  value set. The access_type member should be set to "write". 
        \param comp_id        (\c string)             The composition id
        \param json_variables (\c list[VariableInfo]) Json-encode variables
        \return          (\c ReturnValue)        Value member is empty
        """
        if not self.__bm_processes.has_key(comp_id):
            msg = "no composition with the given id exists, can't write variables"
            ReturnValue(ReturnValue.CODE_SUCCESS, msg, None)
        variables = jsonpickle.decode(json_variables)
        port = self.__bm_processes[comp_id].get_port()
        url = "http://localhost:" + str(port) + "/"
        proxy = xmlrpclib.ServerProxy(url)
        pickled_variables = pickle.dumps(variables)
        r = pickle.loads(proxy.write_variables(pickled_variables))
        return jsonpickle.encode(r)

    def get_listening_port(self):
        return self.__listening_port

    def __find_open_port(self):
        port = None
        while(1):
            port = random.randint(40000, 65535)
            cmd = "netstat -a | egrep 'Proto|LISTEN' | grep " + str(port) + " | wc -l"
            if int(commands.getstatusoutput(cmd)[1]) == 0:
                return port

    def __parse_config(self):
        cp = ConfigParser.ConfigParser()        
        cp.read(self.__config)
        self.__logging_dir = cp.get('MAIN', 'logging_dir')        
        self.__tmp_dir = cp.get('MAIN', 'tmp_dir')        
        self.__listening_port = int(cp.get('NETWORK', 'listening_port'))
        self.__block_ext = cp.get('BLOCKS', 'block_ext')
        self.__blocks_path = cp.get('BLOCKS', 'blocks_path')
        self.__bm_proc_exec = cp.get('MAIN', 'bm_process_exec')
        self.__bm_base_path = cp.get('DEFAULT', 'bm_basepath')

    def __init_logging(self):
        self.__logger = logging.getLogger()
        if not os.path.isdir(self.__logging_dir):
            try:
                commands.getstatusoutput("mkdir -p " + self.__logging_dir)
            except:
                print "error while trying to initialize logging!"
                return

        hdlr = logging.FileHandler(self.__logging_dir + "/bmdaemon.log")
        formatter = logging.Formatter('%(asctime)s %(levelname)s %(message)s')
        hdlr.setFormatter(formatter)
        self.__logger.addHandler(hdlr)
        self.__logger.setLevel(logging.INFO)       

########################################################################
#
# MAIN EXECUTION
#
########################################################################
if __name__ == "__main__":
    if len(sys.argv) < 2:
        print "usage: bmdaemon.py [config]"
        os._exit(1)

    config = sys.argv[1]
    bmd = BMDaemon(config)
    port = bmd.get_listening_port()

    reactor.listenTCP(port, server.Site(bmd))
    print "starting bm daemon on port " + str(port)
    reactor.run()
