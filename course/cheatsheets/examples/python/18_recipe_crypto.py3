# python.md §8 — recipe 2024: biggest rise and fall per crypto symbol, and between which values
text = "BTC X100 hello ETH F245 BTC X90 BTC Y150 ETH F100 BTC Z120 Doge A5 EOS B1 EOS B1"
simboli = ["BTC", "Doge", "EOS", "ETH", "XPM"]
vrednosti = {s: [] for s in simboli}               # symbol -> values in file order
w = text.split()
for i, z in enumerate(w):
    if z in vrednosti and i + 1 < len(w) and w[i + 1][1:].isdigit():
        vrednosti[z].append(int(w[i + 1][1:]))

for s in simboli:
    v = vrednosti[s]
    if len(v) < 2:                                 # missing or once: print nothing
        continue
    rast = pad = None                              # each is (difference, from, to)
    for a, b in zip(v, v[1:]):                     # neighbouring pairs
        if b - a > 0 and (rast is None or b - a > rast[0]):
            rast = (b - a, a, b)
        if a - b > 0 and (pad is None or a - b > pad[0]):
            pad = (a - b, a, b)
    r = f"{rast[0]} ({rast[1]} -> {rast[2]})" if rast else "0"
    p = f"{pad[0]} ({pad[1]} -> {pad[2]})" if pad else "0"
    print(f"Valutata {s}: najgolem porast {r}, najgolem pad {p}")
