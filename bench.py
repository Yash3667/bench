"""
Python front end for executing the drive workload benchmarking
tool.

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
path = "/dev/sdb"
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

rread_sz = long(rread_sz)
rwrite_sz = long(rwrite_sz)
sread_sz = long(sread_sz)
swrite_sz = long(swrite_sz)
offset = 0

rread_op = 0
rread_avg = 0.0
rread_list = []
rread_thru = 0.0
rread_std = 0.0

rwrite_op = 0
rwrite_avg = 0.0
rwrite_list = []
rwrite_thru = 0.0
rwrite_std = 0.0

sread_op = 0
sread_avg = 0.0
sread_list = []
sread_thru = 0.0
sread_std = 0.0

swrite_op = 0
swrite_avg = 0.0
swrite_list = []
swrite_thru = 0.0
swrite_std = 0.0

# Random Reads
rread_op = struct.unpack("L", content[offset:offset+8])[0]
offset += 8
rread_avg = struct.unpack("d", content[offset:offset+8])[0]
offset += 8

try:
    rread_thru = float(rread_sz) * (1 / rread_avg) / float(1024*1024)
except Exception:
    rread_thru = 0.0

for i in range(rread_op):
    rread_list.append(struct.unpack("d", content[offset:offset+8])[0])
    offset += 8
rread_std = numpy.std(numpy.array(rread_list), ddof=1)
print rread_op, rread_avg, rread_thru, rread_std
#print

# Random Writes
rwrite_op = struct.unpack("L", content[offset:offset+8])[0]
offset += 8
rwrite_avg = struct.unpack("d", content[offset:offset+8])[0]
offset += 8

try:
    rwrite_thru = float(rwrite_sz) * (1 / rwrite_avg) / float(1024*1024)
except Exception:
    rwrite_thru = 0.0

for i in range(rwrite_op):
    rwrite_list.append(struct.unpack("d", content[offset:offset+8])[0])
    offset += 8
rwrite_std = numpy.std(numpy.array(rwrite_list), ddof=1)
print rwrite_op, rwrite_avg, rwrite_thru, rwrite_std
#print

# Sequential Reads
sread_op = struct.unpack("L", content[offset:offset+8])[0]
offset += 8
sread_avg = struct.unpack("d", content[offset:offset+8])[0]
offset += 8

try:
    sread_thru = float(sread_sz) * (1 / sread_avg) / float(1024*1024)
except Exception:
    sread_thru = 0.0

for i in range(sread_op):
    sread_list.append(struct.unpack("d", content[offset:offset+8])[0])
    offset += 8
sread_std = numpy.std(numpy.array(sread_list), ddof=1)
print sread_op, sread_avg, sread_thru, sread_std
#print

# Sequential Writes
swrite_op = struct.unpack("L", content[offset:offset+8])[0]
offset += 8
swrite_avg = struct.unpack("d", content[offset:offset+8])[0]
offset += 8

try:
    swrite_thru = float(swrite_sz) * (1 / swrite_avg) / float(1024*1024)
except Exception:
    swrite_thru = 0.0

for i in range(swrite_op):
    swrite_list.append(struct.unpack("d", content[offset:offset+8])[0])
    offset += 8
swrite_std = numpy.std(numpy.array(swrite_list), ddof=1)
print swrite_op, swrite_avg, swrite_thru, swrite_std
#print

latencies = [rread_avg, rwrite_avg, sread_avg, swrite_avg]
thru = [rread_thru, rwrite_thru, sread_thru, swrite_thru]

for i in range(len(latencies)):
    latencies[i] = round(latencies[i], 5)
    thru[i] = round(thru[i], 5)

trace0 = go.Bar(
    x=['Random Read', 'Random Write', 'Sequential Read', 'Sequential Write'],
    y=latencies,
    name='Avg. Latencies (seconds)',
    marker=dict(
        color='rgb(49,130,189)'
    ),
    text=latencies,
    textposition='auto',
    opacity=0.85,
)
trace1 = go.Bar(
    x=['Random Read', 'Random Write', 'Sequential Read', 'Sequential Write'],
    y=thru,
    name='Avg. Throughput (MB/s)',
    marker=dict(
        color='rgb(204,204,204)',
    ),
    text=thru,
    textposition='auto',
    opacity=0.85,
)

data = [trace0, trace1]
layout = go.Layout(
    xaxis=dict(tickangle=-45),
    yaxis=dict(
        type='log',
        autotick=False,
        showgrid=True
    ),
    barmode='group',
    title="Performance Metrics",
)

fig = go.Figure(data=data, layout=layout)
py.plot(fig, filename=args[2])
