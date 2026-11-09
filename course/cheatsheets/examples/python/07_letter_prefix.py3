# python.md §5 — value with a letter prefix (X123): symbol, then the NEXT word, cut off the letter
text = "BTC X100 hello ETH F245 BTC X90 BTC Y150 BTC Z BTC Q7a Doge"
simboli = {"BTC": [], "ETH": [], "Doge": []}
words = text.split()
for i, word in enumerate(words):
    if word in simboli and i + 1 < len(words):       # i+1 < len: the last word has no "next"
        nxt = words[i + 1]
        if len(nxt) > 1 and nxt[1:].isdigit():       # "X100"[1:] is "100"
            simboli[word].append(int(nxt[1:]))
print(simboli)
print(repr("Z"[1:]), "Z"[1:].isdigit())              # an empty string is not a number...
print("Q7a"[1:].isdigit())                           # ...and neither is a mix of digits and letters
