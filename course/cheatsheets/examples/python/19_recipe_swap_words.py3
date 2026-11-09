# python.md §8 — recipe 2025 Lab2 Z1: swap 1st with last word, 2nd with second-last... and write back
import os
import sys
import tempfile

def preturi(content):
    zb = content.split()
    n = len(zb)
    for i in range(n // 2):                        # only the first half: swapping all would undo itself
        zb[i], zb[n - 1 - i] = zb[n - 1 - i], zb[i]    # Python swaps without a temp variable
    return " ".join(zb)

os.chdir(tempfile.mkdtemp())
with open("a.txt", "w") as f:
    f.write("eden dva tri cetiri pet\n")
if len(sys.argv) < 3:                              # the task wants at least two file names
    print("pomalku od 2 argumenti, koristam primer", file=sys.stderr)
    sys.argv = ["z1.py3", "a.txt", "nema.txt"]
for ime in sys.argv[1:]:
    try:
        with open(ime) as f:
            nov = preturi(f.read())
        with open(ime, "w") as f:                  # second open: only after the first is closed
            f.write(nov)
        print("Succes in edit:", ime)
    except FileNotFoundError:
        print("Ne go najdovme fajlot:", ime)
with open("a.txt") as f:
    print(f.read())
print(preturi("a b a"), "|", [x for x in preturi("a b c d").split()])
