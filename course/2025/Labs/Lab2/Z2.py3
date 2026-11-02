#!/usr/bin/python3

import os
import sys
from collections import defaultdict

if len(sys.argv) != 3:
    print("Nevaliden broj na argumenti!")
    sys.exit(1)
    
if os.path.isfile(sys.argv[1]):
    with open(sys.argv[1],"r") as f:
        lines = f.readlines()
    
    count_D = defaultdict(int)
    dates = set()
    
    for line in lines:
        delovi = line.split()
        if len(delovi)>=9:
            den = delovi[6]
            mesec = delovi[5]
            datum = f"{den} {mesec}"
            dates.add(datum)
            count_D[datum]+=1

    if sys.argv[2] == "-f":
        print("Sledi lista na datumi so nivna cestost")
        for dat,count in sorted(count_D.items()):
            print(f"{dat} {count}")
    elif sys.argv[2] == "-d":
        print("Razlicni datumi na kreacija: \n")
        for dat in sorted(dates):
            print(dat)
    else:
        print("Vnesovte nevalidna komanda")
        sys.exit(1)
else:
    print("Ne vnesovte datoteka!")
    sys.exit(1)
