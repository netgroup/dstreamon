
import random
import sys

__hosts = range(1, 250)

def generate(prefix, count):

    ls = []

    random.shuffle(__hosts)

    taked = __hosts[:count]

    for h in taked:
        ls.append( prefix + str(h) )

    return "\n".join(ls)


if __name__=="__main__":

    args = sys.argv
    
    argc = len(args)

    if (argc < 2):
        print "Syntax: python2 generate_hosts.py <count>\n"
        quit()

    count  = int(args[1])
    subs = int(args[2])
    
    with open('host_prefix') as pfile:
       	prefix0 = pfile.readline().strip('\n')
    
    for i in range(subs):
	prefix = prefix0+str(i+1)+"." 
    	ls = generate(prefix, count)

    	print ls
