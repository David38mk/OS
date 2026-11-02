#!/usr/bin/python3
import os
import sys

print("Vo ovaa skripta proveruvame dali eden file e vsusnost file")
f = sys.argv[1]
if os.path.isfile(f):
    print("Datotekata e file")
else:
    print("Zhalam")
