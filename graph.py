import matplotlib.pyplot as plt
import numpy as np

content = ""
with open("test", "r") as this_file:
    content = this_file.read()

index = []
content = content.split("\n")[:-1]
for i in range(len(content)):
    content[i] = float(content[i])
    index.append(i+1)

print sum(content)/len(content)

plt.plot(index, content)
plt.xlabel('Count')
plt.ylabel('Value')
plt.show()
