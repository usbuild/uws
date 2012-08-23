#-*- coding:utf-8 -*-
import os
import time
output = os.popen('ps -e | grep uws')
procs = []
for line in output:
    proc = line.split(None)
    procs.append(proc[0])
fds = [(pid, '/proc/' + pid + '/fd') for pid in procs]
final = {}

while True:
    for (pid, fd) in fds:
        if os.access(fd, os.F_OK):
            num = len(os.listdir(fd))
            print str(num) + "\t",
        else:
            print "*\t",
    print ''
    time.sleep(0.5)
