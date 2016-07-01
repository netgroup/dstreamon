
from subprocess import Popen
import os
import json
import sys
import signal

STREAMON_PATH = os.path.dirname(os.getcwd()) + "/streamon-prototype/"

#STREAMON_PATH = DIR+"/../../" #os.environ.get('STREAMON_ROOT', '../..')
#print(os.path.isfile(STREAMON_PATH + "/config/config_1.xml"))

STREAMON_BIN = './start.sh'

STREAMON_CONFDIR = 'config/'

STREAMON_PIDFILE = STREAMON_PATH + 'streamon.pid'

print('Streamon Root is =>', STREAMON_PATH)

def get_configurations() -> str:
    for dirname, dirnames, filenames in os.walk(STREAMON_PATH + STREAMON_CONFDIR):
        return filenames
       
def put_configurations(body) -> str:
    config_name = body['name']
    config_data = body['data']

    config_filepath = STREAMON_PATH + STREAMON_CONFDIR + config_name

    print("config_filepath:", config_filepath)

    print("config_data:", config_data)

    if not os.path.isfile(config_filepath):
        return 'config {0} not exists'.format(config_name)

    f = open(config_filepath, 'w')

    f.write(config_data)

    f.close()
    
    return 'config {0} succesfully written'.format(config_name)

def post_configurations(body) -> str:
    config_name = body['name']
    config_data = body['data']

    config_filepath = STREAMON_PATH + STREAMON_CONFDIR + config_name

    print("config_filepath:", config_filepath)

    print("config_data:", config_data)

    if os.path.isfile(config_filepath):
        return 'config {0} already exists'.format(config_name)

    f = open(config_filepath, 'w')

    f.write(config_data)

    f.close()
    
    return 'config {0} succesfully written'.format(config_name)
    return 'do some magic!'

def delete_configurations(name) -> str:

    config_filepath = STREAMON_PATH + STREAMON_CONFDIR + name

    print("config_filepath:", config_filepath)

    if not os.path.isfile(config_filepath):
        return 'config {0} not exists'.format(name)

    os.remove(config_filepath)

    return 'config {0} succesfully removed.'.format(name)

def get_commands() -> str:
    return 'do some magic!'

def post_commands_start(body) -> str:

    if os.path.isfile(STREAMON_PIDFILE):
        return 'streamon is currently running'

    config_name = body['name']

    if not config_name:
        return 'provided config name not valid'

    config_name = STREAMON_PATH + STREAMON_CONFDIR + config_name + '.xml'
    print(config_name)

    if not os.path.isfile(config_name):
        return 'config file not found'

    print('config path select:', config_name)

    _streamon_process = Popen([STREAMON_BIN, config_name], cwd=STREAMON_PATH)
    # rc = _streamon_process.wait()

    return 'streamon succesfully started.'

def post_commands_stop(body) -> str:

    if not os.path.isfile(STREAMON_PIDFILE):
        return 'streamon is not currently running'

    pidfile = open(STREAMON_PIDFILE)

    pidstr = pidfile.readline()

    print('read pidfile string:', pidstr)

    pid = int(pidstr)

    os.kill(pid, signal.SIGINT)
    
    return 'streamon succesfully stopped'

def get_parameters() -> str:
    return 'do some magic!'

def parameters_name_get(name) -> str:
    return 'do some magic!'

def parameters_name_put(name) -> str:
    return 'do some magic!'

def parameters_name_post(name) -> str:
    return 'do some magic!'
