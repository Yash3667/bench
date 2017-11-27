"""
Python front end for executing the drive workload benchmarking
tool.

Author: Yash Gupta <yash_gupta12@live.com>
Copyright: Yash Gupta

License: MIT Public License
"""

import sys
import subprocess

name = "./build/bench"
path = ""
rread_prob = 0
rwrite_prob = 0
sread_prob = 0
swrite_prob = 0
rread_sz = 0
rwrite_sz = 0
sread_sz = 0
swrite_sz = 0
timer = 0
interval = 0

args = sys.argv
if len(args) < 2:
    exit(1)

content = ""
with open(args[1], "r") as f:
    content = f.read()

content = content.split("\n")
for item in content:
    info = item.split("=")
    
    if "DRIVE_PATH" in info[0]:
        path = info[1]
    elif "RANDOM_READ_PROB" in info[0]:
        rread_prob = info[1]
    elif "RANDOM_WRITE_PROB" in info[0]:
        rwrite_prob = info[1]
    elif "SEQUENTIAL_READ_PROB" in info[0]:
        sread_prob = info[1]
    elif "SEQUENTIAL_WRITE_PROB" in info[0]:
        swrite_prob = info[1]
    elif "RANDOM_READ_SIZE" in info[0]:
        rread_sz = info[1]
    elif "RANDOM_WRITE_SIZE" in info[0]:
        rwrite_sz = info[1]
    elif "SEQUENTIAL_READ_SIZE" in info[0]:
        sread_sz = info[1]
    elif "SEQUENTIAL_WRITE_SIZE" in info[0]:
        swrite_sz = info[1]
    elif "TIMER" in info[0]:
        timer = info[1]
    elif "INTERVAL" in info[0]:
        interval = info[1]
    else:
        print "Error [File Parsing] " + info[0]
        exit(1)

print path
print "  Random Read Probability......." + rread_prob + "%"
print "  Random Write Probability......" + rwrite_prob + "%"
print "  Sequential Read Probability..." + sread_prob + "%"
print "  Sequential Write Probability.." + swrite_prob + "%"
print "  Random Read Size.............." + rread_sz + " bytes"
print "  Random Write Size............." + rwrite_sz + " bytes"
print "  Sequential Read Size.........." + sread_sz + " bytes"
print "  Sequential Write Size........." + swrite_sz + " bytes"
print "  Timer........................." + timer + " seconds"
print "  Interval......................" + interval + " seconds"

command = name + " "
command += rread_prob + " " + rwrite_prob + " " + sread_prob + " " + swrite_prob + " "
command += rread_sz + " " + rwrite_sz + " " + sread_sz + " " + swrite_sz + " "
command += timer + " " + interval + " " + path

print
process = subprocess.check_call(command, shell=True) 

# Build graph
