# python.md §4 — list, dict counter, setdefault/get, defaultdict, Counter, set, sorted, enumerate, zip, max/min
from collections import defaultdict, Counter
zb = "a b a c b a d".split()
zb.append("e"); zb.insert(0, "z"); zb.remove("z"); print(zb, len(zb), zb[-1], zb.index("c"))
cnt = {}
for z in zb:                                   # counter by hand
    if z not in cnt:
        cnt[z] = 0
    cnt[z] += 1
print(cnt)
c2 = {}
for z in zb:
    c2[z] = c2.get(z, 0) + 1                   # get(key, default)
dd = defaultdict(int)                          # missing key starts at 0
for z in zb:
    dd[z] += 1
groups = {}
for i, z in enumerate(zb):                     # enumerate gives (index, item)
    groups.setdefault(z, []).append(i)         # make the list on first use
print(c2 == cnt == dict(dd), groups)
print(Counter(zb).most_common(2))
print(set(zb), len(set(zb)), "a" in set(zb))   # set: unique, no order
pairs = [("x", 2), ("y", 3), ("z", 3), ("w", 1)]
print(sorted(pairs, key=lambda p: p[1]))                  # ascending by number
print(sorted(pairs, key=lambda p: p[1], reverse=True))    # stable: y stays before z
print(max(pairs, key=lambda p: p[1]), min(pairs, key=lambda p: p[1]))
for ime, br in zip(["a", "b"], [1, 2]):                   # walk two lists together
    print(ime, br)
print([p[0] for p in pairs if p[1] > 1])                  # list comprehension
print(dict.fromkeys(["b", "a", "b"]))                     # dedupe, keep first-seen order
