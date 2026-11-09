# python.md §8 — recipe 2019 exam_date_1: valid dates first in the line, one output line per date, "_out" file
import os
import tempfile

def valid(s):
    p = s.split("-")
    if len(p) != 3 or [len(x) for x in p] != [2, 2, 4] or not all(x.isdigit() for x in p):
        return False
    d, m, y = map(int, p)
    leap = y % 4 == 0 and (y % 100 != 0 or y % 400 == 0)
    days = [31, 29 if leap else 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]
    return 1 <= m <= 12 and 1 <= d <= days[m - 1]

os.chdir(tempfile.mkdtemp())
with open("in.txt", "w") as f:
    f.write("rodenden 03-05-2020 vo 31-04-2021 i 29-02-2024 kraj\nbez datum\n")
ime = "in.txt"
with open(ime) as src, open(f"{ime}_out", "w") as dst:
    for line in src:
        words = line.split()
        dates = []
        for w in words:
            if valid(w) and w not in dates:        # distinct valid dates, in order
                dates.append(w)
        rest = [w for w in words if not valid(w)]  # the other words, order unchanged
        for d in dates:
            dst.write(" ".join([d] + rest) + "\n")
with open("in.txt_out") as f:
    print(f.read(), end="")
trazen = "03-05-2020"                                # the optional 2nd argument, if it is a valid date
if valid(trazen):
    with open("in.txt") as f:
        print("na ekran:", [l.strip() for l in f if trazen in l.split()])
