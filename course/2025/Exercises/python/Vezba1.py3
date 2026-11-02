#!/usr/bin/python3
import sys
import os
for a in sys.argv:
    if os.path.isfile(a):
        f = open(a,"r")
        print(f.readline())
        j = (i.split(".") for i in f)
            print(j[0])
