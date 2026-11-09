# python.md §9 — pitfalls demonstrated: != on a list, int("abc"), readlines newline, read twice, mutate, str vs int, CRLF, is
import io

argv = ["prva.py3", "zbor", "l", "dat.txt"]
print(argv != 4, len(argv) != 4)                   # list != int is ALWAYS True; compare len()
for t in ["42", "abc"]:
    print(t, t.isdigit())                          # test before int(), or catch ValueError
print(argv[len(argv) - 1], argv[1:])               # argv[0] is the script: first real arg is argv[1]
f = io.StringIO("a\nb\n")
for line in f.readlines():
    print(line)                                    # prints "a", blank, "b", blank: line has its own \n
f = io.StringIO("a\nb\n")
print(len(f.readlines()), len(f.readlines()))      # second read finds nothing: the file is at its end
nums = [1, 2, 3, 4, 5, 6]
for n in nums:
    if n % 2 == 0:
        nums.remove(n)                             # skips items: the list shifts under the loop
print(nums)                                        # [1, 3, 5] only by luck; build a new list instead
print("10" == 10, "10" > "9", int("10") > int("9"))    # str vs int never equal; text sorts by characters
print(repr("a b\r\n".split()[-1]), repr("a b\r\n".rstrip("\n")))   # split() eats \r, rstrip("\n") does not
a = [1]
b = [1]
print(a == b, a is b)                           # == same value, is same object: use == for values
