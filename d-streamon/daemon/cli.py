"""\brief Blockmon's CLI, run it with: sudo python cli.py. At the prompt type 
          help for a list of commands.
"""
import os
import sys
from core.bmprocess import BMProcessManager
from core.returnvalue import *
from core.block import *
from core.bmlogging import setup_logging
setup_logging(os.getcwd() + "/cli.log")
from core.bmlogging import bm_logger

cmd = None
args = []
mngr = BMProcessManager(bm_logger=bm_logger)
is_running = False

while (1):
        
    inp = raw_input('BM shell:')
    cmds = inp.split()
    if len(cmds) < 1: continue
    cmd = cmds[0]
    if(len(cmds) > 1):
        args = cmds[1:]

    if(cmd == 'exit'):
        if is_running:
            print "stopping composition"
            r = mngr.stop_composition()
            if r.get_code() != ReturnValue.CODE_SUCCESS:
                print 'error while stopping: ' + str(r.get_msg())
        break

    elif(cmd == 'start'):
        if (mngr.is_running()):
            print 'uninstall blockmon first'
        else:
            try:
                f = open(args[0], "r")
                comp = f.read()
                f.close()
            except:
                print 'error while trying to open file'
                continue
            r = mngr.start_composition(comp)
            if r.get_code() == ReturnValue.CODE_SUCCESS:
                is_running = True
                print 'successfully started composition'
            else:
                print 'error while starting composition: ' + str(r.get_msg())

    elif(cmd == 'stop'):
        if not is_running:
            print 'no composition is running'
            continue
        r = mngr.stop_composition()
        if r.get_code() == ReturnValue.CODE_SUCCESS:
            is_running = False
            print 'done.'
        else:
            print 'error while stopping: ' + str(r.get_msg())

    elif(cmd == 'update'):
        if not is_running:
            print 'no composition is running, cannot update'
            continue
        try:
            f = open(args[0], "r")
            comp = f.read()
            f.close()
        except:
            print 'error while trying to open file'
            continue
        r = mngr.update_composition(comp)
        if r.get_code() == ReturnValue.CODE_SUCCESS:
            print 'successfully updated ' + str(args[0])
        else:
            print 'error while updating ' + str(args[0])

    elif(cmd == 'read'):
        if not is_running:
            print 'no composition is running'
            continue

        if(len(args) > 1):
            block_name = args[0]
            var_names = args[1:]
            variables = []
            for var_name in var_names:
                variables.append(VariableInfo(block_name, var_name, "", "read"))
            r = mngr.read_variables(variables)
            if r.get_code() == ReturnValue.CODE_SUCCESS:
                string = ""
                for var in r.get_value():
                    string += var.get_name() + "=" + str(var.get_value()) + " "
                print string
            else:
                print 'error while reading variable: ' + str(r.get_msg())
        else:
                print 'wrong number of args'

    elif(cmd == 'write'):
        if not is_running:
            print 'no composition is running'
            continue
        if(len(args) == 3):
            block_name = args[0]
            var_name = args[1]
            var_value = args[2]
            variables = [VariableInfo(block_name, var_name, None, "write", var_value)]
            r = mngr.write_variables(variables)
            if r.get_code() == ReturnValue.CODE_SUCCESS:                
                var = r.get_value()
                print 'successfully wrote to variable ' + var_name
            else:
                print 'error while writing variable: ' + str(r.get_msg())
        else:
                print 'wrong number of args'

    elif (cmd == 'help'):
        print "usage: commands [arguments]\n"
        print "start\t[path to composition]"
        print "update\t[path to composition]"
        print "read\t[blockname] [varname1] [varname2]..."
        print "write\t[blockname] [varname] [varvalue]"
        print "stop"
        print "exit\n"

    else:
        print cmd + ': command unknown'
