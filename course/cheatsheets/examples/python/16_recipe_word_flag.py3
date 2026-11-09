# python.md §8 — recipe 2022: <word> <l|w|f> <file>; l = line numbers, w = words before it, f = count
import os
import sys
import tempfile

def main(argv):
    if len(argv) != 4:                       # len(argv), not argv != 4
        print("Nevaliden broj na argumenti", file=sys.stderr)
        return 1
    zbor, opcija, ime = argv[1], argv[2], argv[3]
    if opcija not in ("l", "w", "f"):
        print("Nevalidna opcija", file=sys.stderr)
        return 1
    linii, pred, brojac, najden = [], 0, 0, False
    with open(ime) as f:
        for br, line in enumerate(f, start=1):        # line numbers from 1
            for w in line.split():
                if w == zbor:
                    najden = True
                    brojac += 1
                    if br not in linii:
                        linii.append(br)
                elif not najden:
                    pred += 1                          # words seen before the FIRST match
    if opcija == "l":
        print(" ".join(map(str, linii)) if linii else -1)
    elif opcija == "w":
        print(pred if najden else 0)
    else:
        print(brojac)
    return 0

os.chdir(tempfile.mkdtemp())
with open("t.txt", "w") as f:
    f.write("a b c\nd bb a\nx\na a\n")
for o in ["l", "w", "f", "q"]:
    print(o, "->", end=" ")
    main(["prva.py3", "a", o, "t.txt"])
main(["prva.py3", "zzz", "l", "t.txt"])
sys.exit(main(["prva.py3", "a"]))
