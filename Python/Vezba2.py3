#!/usr/bin/python
import sys
import os
    if os.path.isfile(sys.argv[1]) and os.path.isfile(sys.argv[2]) and os.path.isfile[3]:
        f1 = open(sys.argv[1],"r")
        f2 = open(sys.argv[2],"r")
        f3 = open(sys.argv[3],"w")
        d1 = f1.read()
        d2 = f2.read()
        if len(d1) > len(d2):
            min = d2
            max = d1
        else
            min = d1
            max = d2
        i = 0
        for l1 in min:
            f3.write(l1)
            f3.write(max[i])
            print(l1)
            print(max[i])
            i++
        while i < len(max)
            f3.write(max[i])
            print(max[i])
