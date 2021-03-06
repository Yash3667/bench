"""
Python front end for executing the drive workload benchmarking
tool. Incredibly crappy code which someone (me) should fix.

Author: Yash Gupta <yash_gupta12@live.com>
Copyright: Yash Gupta

License: MIT Public License
"""

import sys
import subprocess
import struct
import numpy
import plotly.plotly as py
import plotly.graph_objs as go

import matplotlib
matplotlib.use("agg")
import matplotlib.pyplot as plt


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

command = "nice -19 " + name + " "
command += rread_prob + " " + rwrite_prob + " " + sread_prob + " " + swrite_prob + " "
command += rread_sz + " " + rwrite_sz + " " + sread_sz + " " + swrite_sz + " "
command += timer + " " + interval + " " + path

print
process = subprocess.check_call(command, shell=True) 

# Build graph
path = path[path.rfind('/')+1:] + ".bin"
content = ""

try:
    with open(path, "rb") as f:
        content = f.read()
except Exception:
    print "Falling back to 'default_output.bin'"
    path = "default_option"
    with open("default_output.bin", "rb") as f:
        content = f.read()

rread_sz = float(rread_sz)
rwrite_sz = float(rwrite_sz)
sread_sz = float(sread_sz)
swrite_sz = float(swrite_sz)
offset = 0

rread_op = 0
rread_avg = 0.0
rread_thru = 0.0
rread_total = 0.0

rwrite_op = 0
rwrite_avg = 0.0
rwrite_thru = 0.0
rwrite_total = 0.0

sread_op = 0
sread_avg = 0.0
sread_thru = 0.0
sread_total = 0.0

swrite_op = 0
swrite_avg = 0.0
swrite_thru = 0.0
swrite_total = 0.0

# Random Reads
rread_op = struct.unpack("L", content[offset:offset+8])[0]
offset += 8
rread_avg = struct.unpack("d", content[offset:offset+8])[0]
offset += 8
rread_total = struct.unpack("d", content[offset:offset+8])[0]
offset += 8

try:
    rread_thru = (rread_sz / rread_avg) / 1048576.0
except Exception:
    rread_thru = 0.0
print rread_op, rread_avg, rread_total, rread_thru

# Random Writes
rwrite_op = struct.unpack("L", content[offset:offset+8])[0]
offset += 8
rwrite_avg = struct.unpack("d", content[offset:offset+8])[0]
offset += 8
rwrite_total = struct.unpack("d", content[offset:offset+8])[0]
offset += 8

try:
    rwrite_thru = (rwrite_sz / rwrite_avg) / 1048576.0
except Exception:
    rwrite_thru = 0.0
print rwrite_op, rwrite_avg, rwrite_total, rwrite_thru

# Sequential Reads
sread_op = struct.unpack("L", content[offset:offset+8])[0]
offset += 8
sread_avg = struct.unpack("d", content[offset:offset+8])[0]
offset += 8
sread_total = struct.unpack("d", content[offset:offset+8])[0]
offset += 8

try:
    sread_thru = (sread_sz / sread_avg) / 1048576.0
except Exception:
    sread_thru = 0.0
print sread_op, sread_avg, sread_total, sread_thru

# Sequential Writes
swrite_op = struct.unpack("L", content[offset:offset+8])[0]
offset += 8
swrite_avg = struct.unpack("d", content[offset:offset+8])[0]
offset += 8
swrite_total = struct.unpack("d", content[offset:offset+8])[0]
offset += 8

try:
    swrite_thru = (swrite_sz / swrite_avg) / 1048576.0
except Exception:
    swrite_thru = 0.0
print swrite_op, swrite_avg, swrite_total, swrite_thru

latencies = [rread_avg, rwrite_avg, sread_avg, swrite_avg]
thru = [rread_thru, rwrite_thru, sread_thru, swrite_thru]

for i in range(len(latencies)):
    latencies[i] = round(latencies[i], 5)
    thru[i] = round(thru[i], 5)

x = ['Random Read', 'Random Write', 'Sequential Read', 'Sequential Write']
x_pos = numpy.arange(len(x))

plt.bar(x_pos, thru, align='center', alpha=0.5)
plt.xticks(x_pos, x)
plt.ylabel("MB/s")
plt.title("Average Throughput - " + args[2])
plt.savefig(args[2] + "-thru")
plt.clf()

plt.bar(x_pos, latencies, align='center', alpha=0.5, color='red')
plt.xticks(x_pos, x)
plt.ylabel("seconds")
plt.title("Average Latency - " + args[2])
plt.savefig(args[2] + "-lat")
