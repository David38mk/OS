# python.md §2 — files: modes, with, read/readlines/for, strip, "_out" file, exists, FileNotFoundError
import os
import tempfile

os.chdir(tempfile.mkdtemp())            # work in a throw-away directory
with open("in.txt", "w") as f:          # "w" creates or empties the file
    f.write("prva linija\nvtora linija\n\ntreta\n")
with open("in.txt", "a") as f:          # "a" appends at the end
    f.write("cetvrta\n")

with open("in.txt") as f:               # "r" (read) is the default mode
    print(repr(f.read()))               # whole file as ONE string
with open("in.txt") as f:
    print(f.readlines())                # list of lines, each still ends with "\n"
with open("in.txt") as f:
    for line in f:                      # one line at a time
        print(repr(line.rstrip("\n")))  # rstrip("\n") removes only the newline

name = "in.txt"
with open(name) as src, open(f"{name}_out", "w") as dst:   # 2019 style: <name>_out
    for line in src:
        dst.write(line.upper())
print(os.path.exists("in.txt_out"), os.path.isfile("in.txt_out"), os.path.isfile("."))
with open("in.txt_out") as f:
    print(f.read().split("\n")[:2])

try:
    open("nema.txt")
except FileNotFoundError:
    print("Ne go najdovme fajlot: nema.txt")
