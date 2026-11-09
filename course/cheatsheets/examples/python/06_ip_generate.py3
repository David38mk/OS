# python.md §5 — IP x.x.x.x validate, mask x:x:x:x, generate every address from IP up to 255
from itertools import product, islice

def parse(word, sep):
    p = word.split(sep)
    if len(p) != 4 or not all(x.isdigit() and int(x) <= 255 for x in p):
        return None                          # None = "not an IP / not a mask"
    return [int(x) for x in p]

def box(start, end):                         # the 2023 solution: each octet runs start..end
    return product(*[range(start[i], end[i] + 1) for i in range(4)])

for w in ["10.0.0.1", "256.1.1.1", "1.2.3", "1.2.3.x", "255:255:0:0"]:
    print(w, parse(w, "."), parse(w, ":"))

ip = parse("250.254.254.253", ".")
all_up = list(box(ip, [255, 255, 255, 255]))             # flag "a": up to 255.255.255.255
print(len(all_up), ".".join(map(str, all_up[0])), ".".join(map(str, all_up[-1])))
mask = parse("251:255:255:254", ":")
for t in islice(box(ip, mask), 5):                       # flag "o": up to the mask, first 5 only
    print(".".join(map(str, t)))
print(len(list(box(ip, mask))))
