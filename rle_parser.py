import re
import sys

name = sys.argv[1]
output = ""
with open("patterns/" + name, "r") as f:
    raw = f.readlines()[2:]
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
output_list = [x + ('b' * (maxlen - len(x))) for x in output_list]
print(len(output_list))
output = ""
for elem in output_list:
    output += elem + "\n"
print(output)

new_file = open(name + ".txt", "w")
new_file.write(output)