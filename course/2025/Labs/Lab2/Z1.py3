#!/usr/bin/python3
import os
import sys

if len(sys.argv) < 3:
    print("Vnesovte nevaliden broj na argumenti")
    sys.exit(1)

def proMena(content):
    zborovi = content.split()
    N = len(zborovi)
    for i in range(N//2):
        if zborovi[i] != zborovi[N-i-1]:
           pom = zborovi[i]
           zborovi[i] = zborovi[N-i-1]
           zborovi[N-i-1] = pom
    return ' '.join(zborovi)

for i in sys.argv[1:]:
    try:
        with open(i,"r") as f:
            content = f.read()
        
        Mod_content = proMena(content)
        with open(i,"w") as f:
            f.write(Mod_content)
            
        print("Succes in edit: {}".format(i))
    except FileNotFoundError:
        print("Ne go najdovme fajlot: {}".format(i))
        
