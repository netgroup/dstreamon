#!/usr/bin/env python
#NOTE: must u+x this file for things to work with the bm daemon!!!!
import sys
sys.path.append("/home/fhuici/research/eu/demons/svn/Sources/blockmon/main/node/daemon/")
from txjsonrpc.web import jsonrpc
from twisted.web import server
from twisted.internet import reactor
from txjsonrpc.web.jsonrpc import Proxy
import xml.dom.minidom
import xmlrpclib, pickle
from SimpleXMLRPCServer import SimpleXMLRPCServer
from core.returnvalue import *
from core.bmparser import CompositionParser
import os
import random
import commands
from composition import CompositionManager
from core.bmlogging import setup_logging

import imp
blockmon = imp.load_dynamic('blockmon','../libblockmonlib.so')

class BMProcessManager:

    """\brief Controls a running blockmon process. Note that this class/file can be
              used directly as an executable (the method used by the blockmon daemon
              to spawn blockmon processes) or by creating an instance of the class
              (the method used for the blockmon CLI). For the former, the manager runs
              an XML-RPC server which the blockmon daemon uses to communicate with it.
              Further note that all xml-rpc operations return a pickled ReturnValue object.
    """
    bm_running = False
    
    def __init__(self, comp=None, bm_logger=None, port=None, is_comp_str=False):
        """\brief Initializes class
        \param comp       (\c string)         The composition
        \param bm_logger  (\c logging.logger) The bm logger
        \param port       (\c string)         The port to run the xml-rpc server on
        \parm is_comp_str (\c bool)           Whether the composition or a file path
        """
        self.__composition = comp
        if comp and not is_comp_str:
            f = open(comp, "r")
            self.__composition = f.read()
            f.close()

        self.__logger = bm_logger
        self.__port = None
        if port:
            self.__port = int(port)
        self.__server = None

    def set_composition(self, comp):
        self.__composition = comp

    def set_logger(self, logger):
        self.__logger = logger

    def serve(self):
        """\brief Starts up a composition as well as the xml-rpc server
        """
        self.start_composition()
        self.__server = SimpleXMLRPCServer(("localhost", self.__port))
        self.__server.register_function(self.update_composition, "update_composition")
        self.__server.register_function(self.stop_composition, "stop_composition")
        self.__server.register_function(self.read_variables, "read_variables")
        self.__server.register_function(self.write_variables, "write_variables")
        self.__logger.info("Starting Blockmon process with pid=" + str(os.getpid()) +\
                           " and listening on localhost:" + str(self.__port))
        self.__server.serve_forever()

    def start_composition(self, comp=None):
        """\brief Starts up a composition
        \param comp (\c string)      The composition. If None self.__composition is used
        \return     (\c ReturnValue) The result of the operation
        """
        if comp:
            self.__composition = comp
        self.__parser = CompositionParser(self.__composition)
        self.__comp_id = self.__parser.parse_comp_id()
        self.__comp_mngr = CompositionManager(self.__comp_id, blockmon, self.__logger)
        self.__comp_mngr.install(xml.dom.minidom.parseString(self.__composition))
        self.start_bm()
        return ReturnValue(ReturnValue.CODE_SUCCESS, "", None)

    def update_composition(self, comp):
        """\brief Updates up a composition
        \param comp (\c string)      The composition
        \return     (\c ReturnValue) The result of the operation
        """
        self.stop_bm()
        self.__composition = comp
        self.__comp_mngr.reconfigure(xml.dom.minidom.parseString(comp))
        self.start_bm()
        r = ReturnValue(ReturnValue.CODE_SUCCESS, "", None)
        if self.__server:
            return pickle.dumps(r)
        return r

    def is_running(self):
        return self.bm_running

    def stop_composition(self):
        """\brief Stops the composition
        \return     (\c ReturnValue) The result of the operation
        """
        self.stop_bm()
        self.__comp_mngr.remove()
        r = ReturnValue(ReturnValue.CODE_SUCCESS, "", None)
        if self.__server:
            return pickle.dumps(r)
        return r

    def read_variables(self, variables):
        """\brief Reads variables from blocks
        \param variables (\c [VariableInfo]) The variables to read, pickled.
        \return          (\c ReturnValue)    The result of the operation
        """        
        if self.__server:
            variables = pickle.loads(variables)
        for v in variables:
            value = self.__comp_mngr.read_block_var(v.get_block_name(), v.get_name())
            v.set_value(value)
        r = ReturnValue(ReturnValue.CODE_SUCCESS, "", variables)
        if self.__server:
            return pickle.dumps(r)
        return r

    def write_variables(self, variables):
        """\brief Writes values to block variables
        \param variables (\c [VariableInfo]) The variables to write to, pickled.
        \return          (\c ReturnValue)    The result of the operation
        """                
        if self.__server:
            variables = pickle.loads(variables)
        for v in variables:
            self.__comp_mngr.write_block_var(v.get_block_name(),\
                                             v.get_name(),\
                                             v.get_value())
        r = ReturnValue(ReturnValue.CODE_SUCCESS, "", None)
        if self.__server:
            return pickle.dumps(r)
        return r

    @staticmethod
    def start_bm():
        """\brief Starts all blockmon schedulers and timers
        """
        if (BMProcessManager.bm_running):
            raise Exception('blockmon already running')
        else:
            BMProcessManager.bm_running = True
        blockmon.start_schedulers()
        blockmon.start_timer()

    @staticmethod
    def stop_bm():
        """\brief Stops all blockmon schedulers and timers
        """
        if (BMProcessManager.bm_running):
            blockmon.stop_schedulers()
            blockmon.stop_timer()
        BMProcessManager.bm_running = False


class BMProcessInfo:

    """\brief Convenience class for storing information about a running blockmon process
    """

    def __init__(self, proc, comp, logfile, port=None):
        """\brief Initializes class
        \param proc    (\c subprocess.Popen) The process 
        \param comp    (\c string)           The composition XML 
        \param logfile (\c string)           The path to the process' log file
        \param port    (\c int)        The port the process' json-rpc server is running on
        """
        self.__proc = proc
        self.__comp = comp
        self.__logfile = logfile
        self.__port = port

    def get_pid(self):
        if not self.__proc:
            return None
        return self.__proc.pid

    def get_comp(self):
        return self.__comp

    def get_port(self):
        return self.__port

    def set_port(self, p):
        self.__port = p

    def get_logfile(self):
        return self.__logfile
            
    def get_proc(self):
        return self.__proc

    def __str__(self):
        return "BMProcessInfo: pid=" + str(self.get_pid()) + \
                              "port=" + str(self.get_port()) + \
                              "logfile=" + str(self.get_logfile()) + \
                              "\n\tcomposition:\n" + str(self.get_comp())



########################################################################
# MAIN EXECUTION
########################################################################
if __name__ == "__main__":
    if (len(sys.argv) < 3):
        os._exit(1)

    # Setup manager
    compfile = sys.argv[1]
    logfile = sys.argv[2]
    process_port = sys.argv[3]

    setup_logging(logfile)
    from core.bmlogging import bm_logger

    # Start server
    mngr = BMProcessManager(compfile, bm_logger, process_port)
    mngr.serve()
