# python.md §3 — strings: split, strip, startswith, isdigit, join, replace, find/in, slicing, formatting
line = "  BTC X123 12-03-2024 10.0.0.1 -l  \n"
print(line.split())                    # no argument: split on any run of whitespace
print("12-03-2024".split("-"))         # with a separator: exactly that character
print("a,,b".split(","))               # separator keeps empty pieces: ['a', '', 'b']
print(repr(line.strip()))              # removes whitespace (and \n) at both ends
print(repr("xxhixx".strip("x")))       # or the given characters
w = "X123"
print(w.startswith("X"), w.endswith("3"), w[1:].isdigit(), w[1:].isnumeric(), w.isalpha())
print("-".join(["a", "b", "c"]), "a b".replace(" ", "_"))
print("12-03".find("-"), "12-03".find("x"), "-" in "12-03")   # find gives -1 when absent
s = "10.0.0.1"
print(s[0:2], s[3:], s[-1], s[::-1])   # [a:b] stops BEFORE b; negative counts from the end
ime, n = "dat", 3
print(f"{ime} ima {n} reda, {n/2:.2f} prosek")        # f-string: {expr:format}
print("{} ima {} reda".format(ime, n))                # older style, same result
for text in ["42", "4x2", "-7", "3.5", ""]:
    try:
        print(text, "->", int(text))
    except ValueError:                                # int() raises on bad text
        print(text, "-> ne e cel broj")
print(float("3.5"), "-7".isdigit())    # isdigit() is False for a minus sign
