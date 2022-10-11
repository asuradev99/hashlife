import re
import sys
import numpy as np

name = sys.argv[1]
output = ""
with open("patterns/" + name, "r") as f:
    raw = f.readlines()[5:]
    raw = ''.join(raw).replace("\n","").split("$")
    for elem in raw:
        buffer = ""
        for char in elem:
            if char.isalpha():
                num = int(buffer) if buffer != "" else 1
                output += num * char
                buffer = ""
            elif char.isnumeric():
                buffer += char
            elif char == "!":
                break
        output += "\n"
output_list = output.split("\n")
maxlen = len(max(output_list, key=len))

print(maxlen)
output_list = [list(x + ('b' * (maxlen - len(x)))) for x in output_list]


#compute next highest power of two to determine how many b's to zero-pad the output with
x = maxlen
y = len(output_list)
maxsize = max(maxlen, y)
print("Maxsize: ", maxsize)
next =  1<<(maxsize-1).bit_length()
print(next)
print(next - x)
print(next - y)

y_pad = (next - y) // 2
x_pad = (next - x) //2
output_list = np.pad(output_list, ((y_pad,(next - y) - y_pad), (x_pad, (next-x) - x_pad)), mode='constant', constant_values='b')

#print(output_list)


output = ""
for elem in output_list:
    output += ''.join(elem) + "\n"

#print(output)
new_file = open(name + ".txt", "w")
new_file.write(str(next) + "\n")
new_file.write(output)