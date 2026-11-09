# Python Cheat Sheet – study guide for the OS labs and exams

The first-partial exams always contain one Python script task (`Z2.py3`): read a text file, split it into words, pick out some pattern, print or write a result. This sheet covers exactly the Python that task needs, plus the few places where Python meets the shell (exams chain `Z1.sh` -> `Z2.py3`). It assumes you know basic Python; the new part is the exam style.

Read sections 1 to 4 once. Use 5 to 7 as recipes. Section 8 walks through every past task, section 9 lists the mistakes found in the repo's own solutions (kindly, they are all instructive).

Rules of the road:

- Every snippet here exists as a runnable file in `course/cheatsheets/examples/python/` (name given at each section). Run one with `python3 NN_name.py3`.
- Lines starting with `$` are typed at the prompt, the lines under them are real output.
- Macedonian task text is quoted where a recipe comes from a real task. Variable names in the course solutions are Latin transliteration (`zborovi`, `linii`); copy that style, it is what the graders see.
- Syntax of regular expressions is in `shell.md` section 10. Here only the Python calls are covered.

---

## 1. Script skeleton

Concept: `sys.argv` is a list of strings. `argv[0]` is the script name, `argv[1]` the first real argument. Everything is a string, even `5`.

| Name | Type | Meaning | Notes |
|---|---|---|---|
| `sys.argv` | list of str | the command line | `len(sys.argv)` is 1 when no arguments were given |
| `sys.argv[1:]` | list of str | only the real arguments | loop over this for "any number of files" |
| `sys.exit(n)` | function | stop now with exit status `n` | no argument means 0 (success); numbers 1 to 255 mean failure |
| `print(..., file=sys.stderr)` | function | error message on stream 2 | keeps the error out of results that the shell captures |
| `len(x)` | function | number of elements | the way to count arguments |

First line of an executable script is the shebang, `#!/usr/bin/python3`. Two ways to run it:

| Way to run | Needs `chmod +x`? | Notes |
|---|---|---|
| `python3 Z2.py3 a b` | no | always works, the extension does not matter |
| `./Z2.py3 a b` | yes (`chmod +x Z2.py3`) | uses the shebang; this is how a shell script calls it |

Pattern (file `01_argv_skeleton.py3`): required argument, optional number, flag, usage on stderr, exit codes.

```python
import sys

def usage():
    print(f"Upotreba: {sys.argv[0]} <datoteka> [N] [-b]", file=sys.stderr)
    sys.exit(1)

if len(sys.argv) < 2:          # argv[0] is the script name, so 2 means one real argument
    usage()

ime = sys.argv[1]
N = 10                         # default used when the optional argument is missing
blok = False
for arg in sys.argv[2:]:       # everything after the file name, in any order
    if arg == "-b":
        blok = True
    elif arg.isdigit():
        N = int(arg)
    else:
        print(f"Nepoznat argument: {arg}", file=sys.stderr)
        sys.exit(2)
```

```
$ python3 01_argv_skeleton.py3 dat.txt 5 -b
datoteka=dat.txt N=5 -b=True
argv = ['01_argv_skeleton.py3', 'dat.txt', '5', '-b']
$ python3 01_argv_skeleton.py3 ; echo "status $?"
Upotreba: 01_argv_skeleton.py3 <datoteka> [N] [-b]
status 1
```

Which check to use:

| Task wording | Check |
|---|---|
| "exactly three arguments" (2022: word, option, file) | `len(sys.argv) != 4` (script name + 3) |
| "at least two" (2025 Lab 2 Z1: two file names) | `len(sys.argv) < 3` |
| "optional second argument" | `len(sys.argv) > 2` before touching `argv[2]` |
| "flag `a`, optional extra flag `o` followed by a file" (2023) | check `len(sys.argv) < 4` only inside the `o` branch |
| "invalid option: stop" | `print(..., file=sys.stderr)` then `sys.exit(1)` |

Course files that use this: `course/2026/Labs/Lab2/Z1_S.py3` (lines 26 to 41, the same loop as above: file name, optional `N`, optional `-b`), `course/2025/Labs/Lab2/Z2.py3` (`len(sys.argv) != 3`, then flag `-f` or `-d`), `course/2023/Exams/partial_one/Z2.py3` (flag with its own extra argument).

---

## 2. Files

Concept: `open(name, mode)` gives a file object. Use a `with` block so the file is closed for you, even when an error happens.

| Call | Returns | Notes |
|---|---|---|
| `open(name)` or `open(name, "r")` | file object | read; raises `FileNotFoundError` if missing |
| `open(name, "w")` | file object | write; creates the file or **empties** it |
| `open(name, "a")` | file object | append at the end |
| `f.read()` | one str | the whole file, newlines included |
| `f.readlines()` | list of str | every line **still ends with `"\n"`** |
| `for line in f:` | one str per step | same lines, one at a time, no big list |
| `f.readline()` | one str | next line, `""` at the end |
| `f.write(s)` | number of chars | writes exactly `s`; **you** add `"\n"` |
| `line.rstrip("\n")` | str | removes only the newline |
| `os.path.exists(p)` | bool | file or directory exists |
| `os.path.isfile(p)` | bool | exists and is a regular file |
| `os.path.isdir(p)` | bool | exists and is a directory |

Pattern (file `02_files.py3`):

```python
with open("in.txt", "w") as f:          # "w" creates or empties the file
    f.write("prva linija\nvtora linija\n\ntreta\n")

with open("in.txt") as f:
    print(f.readlines())                # list of lines, each still ends with "\n"
with open("in.txt") as f:
    for line in f:                      # one line at a time
        print(repr(line.rstrip("\n")))  # rstrip("\n") removes only the newline

name = "in.txt"
with open(name) as src, open(f"{name}_out", "w") as dst:   # 2019 style: <name>_out
    for line in src:
        dst.write(line.upper())

try:
    open("nema.txt")
except FileNotFoundError:
    print("Ne go najdovme fajlot: nema.txt")
```

Which read to pick:

| Need | Use |
|---|---|
| every word of the file, line breaks do not matter (2024, 2025 Lab 2 Z1) | `words = f.read().split()` |
| the line number or per-line logic (2019 dates, 2022 `l` option, 2023 IPs) | `for line in f:` (or `enumerate(f, start=1)`) |
| the same file twice | read once into a list, loop over the list twice (section 9, pitfall 5) |
| output file named after the input | `f"{name}_out"` |

Course files: `course/2019/Exams/exam_date_1/Z2.py3` (the `_out` file), `course/2025/Labs/Lab2/Z1.py3` (read, change, write back the same name; one `with` per direction, and `FileNotFoundError` caught per file so one missing file does not stop the others), `course/2025/Labs/Lab2/Z2.py3` (`os.path.isfile` before opening), `course/2025/Exercises/python/Proba0.py3` (the smallest `isfile` example).

Beyond this course: `pathlib.Path.read_text()` and the `csv` module do the same jobs; the exams never need them.

---

## 3. Strings

Concept: a string is immutable text. Every method returns a **new** string, so `s.strip()` alone changes nothing; write `s = s.strip()`.

| Method | Returns | Notes |
|---|---|---|
| `s.split()` | list | splits on any run of spaces, tabs, newlines; no empty pieces; also eats the trailing `\n` and `\r` |
| `s.split(sep)` | list | splits on exactly `sep`; keeps empty pieces (`"a,,b".split(",")` is `['a', '', 'b']`) |
| `s.strip()` / `lstrip()` / `rstrip()` | str | removes whitespace at both / left / right end; `strip("x")` removes the given characters |
| `s.startswith(p)` / `endswith(p)` | bool | `w.startswith("-")` is the "is this an argument" test |
| `s.isdigit()` | bool | only characters `0-9`, and not empty. `"-7".isdigit()` is `False` |
| `s.isnumeric()` | bool | same for our purposes; `isdigit` is the usual choice |
| `s.isalpha()` | bool | only letters, and not empty |
| `sep.join(list)` | str | `"-".join(["a","b"])` is `"a-b"`; the list must hold strings |
| `s.replace(a, b)` | str | every `a` replaced by `b` |
| `s.find(x)` | int | index of first `x`, or `-1` |
| `x in s` | bool | substring test; the readable alternative to `find` |
| `s[a:b]` | str | from index `a` up to **but not including** `b`; negative counts from the end; `s[::-1]` reverses |
| `len(s)` | int | length |
| `int(s)` / `float(s)` | number | raises `ValueError` on bad text |
| `s.upper()` / `lower()` | str | case change |

Formatting: f-string `f"{ime} ima {n} reda, {n/2:.2f} prosek"` (preferred; `:.2f` is two decimals), or `"{} ima {} reda".format(ime, n)` (used in 2025 Lab 2). `print(x, end="")` prints without the newline.

Pattern (file `03_strings.py3`), `int()` guarded two ways:

```python
for text in ["42", "4x2", "-7", "3.5", ""]:
    try:
        print(text, "->", int(text))
    except ValueError:                                # int() raises on bad text
        print(text, "-> ne e cel broj")
```

```
42 -> 42
4x2 -> ne e cel broj
-7 -> -7
3.5 -> ne e cel broj
 -> ne e cel broj
```

Rule of thumb: when the task says "a number", test with `isdigit()` **before** `int()` (cheap, no exception), and use `try/except ValueError` when negatives or decimals are possible.

Course files: `course/2024/Exams/partial_one/Z2.py3` (line 29: `next_word[1:].isnumeric()` cuts the leading letter off `X123`), `course/2025/Labs/Lab2/Z1.py3` (`' '.join(zborovi)`), `course/2026/Labs/Lab2/Z1.py3` (`vreme.startswith("[")`, `vreme[1:-3]`).

---

## 4. Collections

Concept: the exams are won by choosing the right container. List for order, dict for "count per name", set for "distinct".

| Operation | Example | Notes |
|---|---|---|
| list: add / insert / remove | `l.append(x)`, `l.insert(0, x)`, `l.remove(x)` | `remove` deletes the first equal item; error if absent |
| list: index and size | `l[0]`, `l[-1]`, `len(l)`, `l.index(x)` | `-1` is the last item |
| dict: read with default | `d.get(k, 0)` | no error when `k` is missing |
| dict: set default | `d.setdefault(k, [])` | creates the value on first use, returns it |
| dict: loop | `for k, v in d.items():` | order = order of first insertion |
| `defaultdict(int)` | `dd[k] += 1` | missing key starts at `0`; `defaultdict(list)` starts at `[]` |
| `Counter(list)` | `Counter(words).most_common(3)` | list of `(item, count)`, biggest first |
| set | `set(l)`, `x in s`, `s.add(x)` | unique items, no order |
| `dict.fromkeys(l)` | `list(dict.fromkeys(l))` | removes duplicates and **keeps first-seen order** |
| `sorted(x, key=f, reverse=b)` | `sorted(d.items(), key=lambda p: p[1])` | returns a new list; the original is untouched |
| `enumerate(x, start=0)` | `for i, w in enumerate(words):` | index and item together |
| `zip(a, b)` | `for a, b in zip(v, v[1:]):` | walks two lists in step; `zip(v, v[1:])` gives neighbouring pairs |
| `max(x, key=f)` / `min` | `max(pairs, key=lambda p: p[1])` | the item with the biggest key |
| list comprehension | `[w for w in words if w.startswith("-")]` | build a filtered list in one line |

**Sorting is stable**: items with equal keys keep their original order. That is how `sorted(per_ip.items(), key=lambda kv: kv[1], reverse=True)` gives "top 3, ties by first appearance" (2026 Lab 2), because `reverse=True` also keeps the original order of equal items. A dict remembers insertion order, so the "original order" is the order of first appearance in the file.

Pattern (file `04_collections.py3`), the counter written by hand and with `defaultdict` (`Counter` exists too, but no course file uses it):

```python
cnt = {}
for z in zb:                                   # by hand: the form the course uses most
    if z not in cnt:
        cnt[z] = 0
    cnt[z] += 1

dd = defaultdict(int)                          # from collections import defaultdict
for z in zb:
    dd[z] += 1                                 # same result, one line shorter
```

```
{'a': 3, 'b': 2, 'c': 1, 'd': 1, 'e': 1}
[('y', 3), ('z', 3), ('x', 2), ('w', 1)]       <- sorted(..., reverse=True): y still before z
```

Course files: `course/2025/Labs/Lab2/Z2.py3` (`defaultdict(int)` as a counter and a `set` of dates), `course/2024/Exams/partial_one/Z2.py3` (dict of lists, one list per currency), `course/2026/Labs/Lab2/Z1_S.py3` (`setdefault(code, [0, 0])` holds two numbers per status; `dict.fromkeys(...)` dedupes the blocklist in order), `course/2026/Labs/Lab2/Z1.py3` (the same without any import, `if k not in d`).

---

## 5. Parsing recipes

Each recipe below is a working building block taken from an exam or lab task.

### 5.1 A date `DD-MM-YYYY`, valid by hand (2019 `exam_date_1`)

Order of checks: shape first (3 pieces of length 2, 2, 4, all digits), then the numbers. The day limit depends on the month, and February depends on the leap year: divisible by 4, except centuries, except centuries divisible by 400. File `05_date_valid.py3`:

```python
M31 = (1, 3, 5, 7, 8, 10, 12)
M30 = (4, 6, 9, 11)

def valid_date(s):
    p = s.split("-")
    if len(p) != 3 or [len(x) for x in p] != [2, 2, 4]:   # shape first: DD-MM-YYYY
        return False
    if not all(x.isdigit() for x in p):                   # before any int(), or int() crashes
        return False
    d, m, y = int(p[0]), int(p[1]), int(p[2])
    leap = y % 4 == 0 and (y % 100 != 0 or y % 400 == 0)
    if m in M31:
        top = 31
    elif m in M30:
        top = 30
    elif m == 2:
        top = 29 if leap else 28
    else:
        return False                                      # month 0 or 13+
    return 1 <= d <= top
```

```
29-02-2024   True      29-02-2023   False     29-02-1900   False     29-02-2000   True
31-04-2024   False     00-01-2024   False     1-1-2024     False     2024-01-31   False
```

### 5.2 IP address `x.x.x.x`, mask, and generating addresses (2023)

An IP is valid when it has 4 parts, every part is digits and at most 255. The task says masks look like `x:x:x:x`, so the same check is called with `":"`. Generating "all addresses from the IP up to the mask" with every octet running from its start value to its end value is a product of four ranges (this is what the nested `for i/j/k/h` loops in the 2023 solution do). File `06_ip_generate.py3`:

```python
def parse(word, sep):
    p = word.split(sep)
    if len(p) != 4 or not all(x.isdigit() and int(x) <= 255 for x in p):
        return None                          # None = "not an IP / not a mask"
    return [int(x) for x in p]

def box(start, end):                         # each octet runs start..end
    return product(*[range(start[i], end[i] + 1) for i in range(4)])
```

```
$ python3 06_ip_generate.py3
10.0.0.1 [10, 0, 0, 1] None
256.1.1.1 None None
255:255:0:0 None [255, 255, 0, 0]
72 250.254.254.253 255.255.255.255      <- flag "a": 72 addresses up to 255.255.255.255
250.254.254.253                          <- flag "o": first five up to the mask 251:255:255:254
...
16
```

Count check by hand: octet ranges 250..251, 254..255, 254..255, 253..254 give 2 x 2 x 2 x 2 = 16.

Careful: from `0.0.0.0` to `255.255.255.255` is 4.3 billion lines. Test with addresses near the top.

### 5.3 A value with a letter prefix, `X123` (2024)

The text is `BTC X100 ...`: the symbol, then the next word is a letter plus digits. Find the symbol with `enumerate`, look at `words[i + 1]`, cut off the first letter, test the rest. File `07_letter_prefix.py3`:

```python
for i, word in enumerate(words):
    if word in simboli and i + 1 < len(words):       # i+1 < len: the last word has no "next"
        nxt = words[i + 1]
        if len(nxt) > 1 and nxt[1:].isdigit():       # "X100"[1:] is "100"
            simboli[word].append(int(nxt[1:]))
```

```
{'BTC': [100, 90, 150], 'ETH': [245], 'Doge': []}
```

`BTC Z` (letter, no digits) and `BTC Q7a` (digits and a letter) are skipped, and no `int()` ever sees bad text.

### 5.4 A command with dash-arguments spanning several lines (2019 `partial_one`)

If the whole text file becomes one flat word list (`f.read().split()`), line breaks disappear and "arguments continue on the next line" costs nothing. Walk the words with an index: on a command, collect the following words that start with `-`, stop at the first one that does not. File `08_cmd_args.py3`:

```python
rez = {}                                          # command -> arguments (dict keeps first-seen order)
i = 0
while i < len(zborovi):
    z = zborovi[i]
    if z in komandi:
        args = []
        i += 1
        while i < len(zborovi) and zborovi[i].startswith("-"):   # stop at first non-dash word
            args.append(zborovi[i])
            i += 1
        if z not in rez:
            rez[z] = args
        elif args:
            rez[z] += ["-e"] + args               # repeat: add to the first one, after "-e"
    else:
        i += 1
```

```
ls -l -a -h -e -R
cp -r
rm
```

(`ls` appears three times; `-h` continues the line before it.) A `while` with a manual index is the right tool when an inner loop must consume words the outer loop should then skip.

### 5.5 `ls -al` lines: date and day (2025 Lab 2 Z2)

An `ls -al` line has 9 fields when the name has no spaces. Counting from 0: permissions, links, owner, group, size, **month (5), day (6)**, time (7), name (8). The first line, `total 20`, has only 2 fields, so `len(fields) >= 9` filters it. File `09_ls_dates.py3`:

```python
for line in ls_out.splitlines():
    f = line.split()
    if len(f) < 9:                         # skips the "total 20" line
        continue
    count[f"{f[6]} {f[5]}"] += 1           # "3 Nov"
```

```
-d:                    -f:
3 Nov                  28 Oct 2
28 Oct                 3 Nov 3
```

Note the `-f` order: pairs are sorted as **text**, so `"28 Oct"` comes before `"3 Nov"`. Fine for the task as written; a real calendar order would need the month turned into a number.

### 5.6 Access log with the 10-word layout (2026 Lab 2)

A valid line has exactly 10 words after `split()`: `0 ip, 1 -, 2 -, 3 [date:time, 4 +0100], 5 "GET, 6 /path, 7 HTTP/1.1", 8 status, 9 bytes`. Anything else is skipped. The minute is word 3 without the leading `[` and without the trailing `:SS`: `w[3][1:-3]`. File `10_access_log.py3`:

```python
status = {}                                   # code -> [requests, bytes]
for line in log:
    w = line.split()
    if len(w) != 10 or not w[8].isdigit() or not w[9].isdigit():
        continue                              # does not follow the format: skip
    minuta = w[3][1:-3]                       # "[03/Nov/2025:15:17:02" -> "03/Nov/2025:15:17"
    st = status.setdefault(int(w[8]), [0, 0])
    st[0] += 1
    st[1] += int(w[9])
```

```
Status 200: 2 baranja, 1555 bajti
Status 401: 1 baranja, 231 bajti
03/Nov/2025:15:17
```

Full solution: `course/2026/Labs/Lab2/Z1.py3`. The weakness of this layout: a request path with a space (`"GET /a b.html HTTP/1.1"`) makes 11 words, so the line is skipped. Section 6 shows the regex that does not have this problem.

---

## 6. Regular expressions in Python

Concept: `re` finds patterns inside text. Use it when the pattern has a **shape** (digits here, a bracket there) rather than a position. The pattern syntax (`\d`, `{2}`, `[^"]`, groups) is explained in `shell.md` section 10; Python uses the same ERE-style symbols plus `\d`, `\w`, `\s`, which work in Python but **not** in `grep -E`.

| Function | Returns | Notes |
|---|---|---|
| `re.match(p, s)` | match object or `None` | pattern must fit at the **start** of `s` |
| `re.search(p, s)` | match object or `None` | pattern may be anywhere |
| `re.fullmatch(p, s)` | match object or `None` | the whole string must fit |
| `re.findall(p, s)` | list of str (or of tuples when `p` has groups) | all non-overlapping matches |
| `re.sub(p, repl, s)` | str | replace every match |
| `re.compile(p)` | pattern object | same methods (`pat.match(s)`); compile once before a loop |
| `m.group()` / `m.group(1)` | str | the whole match / the first `()` |
| `m.groups()` | tuple | all groups |

**Always write the pattern as a raw string, `r"..."`.** In a normal string `"\d"` is a Python escape; in `r"\d"` the backslash reaches `re` untouched.

Always test the result before using it: `m = pat.match(line)` is `None` when the line does not fit, and `None.group(1)` crashes.

File `11_regex.py3`:

```python
line = '10.0.0.7 - - [03/Nov/2025:15:17:02 +0100] "GET /index.html HTTP/1.1" 200 1043'
print(re.match(r"GET", line))                       # None, "GET" is not at the start
print(re.search(r"GET", line).start())              # search: anywhere in the string
print(re.findall(r"\d{2}-\d{2}-\d{4}", "a 01-02-2024 b 31-12-1999"))
m = re.search(r"\[(\d{2})/(\w{3})/(\d{4})", line)   # () = capture group
print(m.group(0), m.groups(), m.group(2))
pat = re.compile(r"^(\S+) .* (\d{3}) (\d+)$")       # compile once, reuse in a loop
m = pat.match(line)
print(m.group(1), int(m.group(2)), int(m.group(3)))
```

```
None
43
['01-02-2024', '31-12-1999']
[03/Nov/2025 ('03', 'Nov', '2025') Nov
10.0.0.7 200 1043
```

### Regex or split?

The course ships both versions of the access-log task: `course/2026/Labs/Lab2/Z1_S.py3` (regex, 95 lines) and `Z1.py3` (split, 122 lines). They print identical output. File `12_regex_vs_split.py3` runs both ideas on two lines, the second with a space in the path:

```
10 ('10.0.0.7', '200', '1043') ('10.0.0.7', '200', '1043')
11 None ('10.0.0.8', '200', '5')
```

| | `split()` | `re` |
|---|---|---|
| Needs | fixed word positions | a pattern |
| Handles `"GET /a b HTTP/1.1"` (quoted text with spaces) | no: 11 words, line skipped | yes: `"[^"]*"` swallows the whole quoted part |
| Validation | by hand (`isdigit`, `len`) | one pattern checks the full shape |
| Readability for a beginner | high | lower, one dense line |
| Good for | "words separated by spaces" tasks (nearly all exams say this) | formats with brackets, quotes or optional parts |

Exam advice: the exam texts nearly always say "words are separated by a space". Then `split()` is enough, shorter to write, and easier to debug under time pressure. Reach for `re` when a field can contain the separator.

---

## 7. Processes from Python

Python can start other programs. Exams use it rarely, but the lab does (Lab 2 is called "Python and processes"), and exams chain shell and Python in one direction: the shell script calls the Python script and uses its output and exit status.

| Call | Returns | Notes |
|---|---|---|
| `subprocess.run(["ls", "-l"])` | `CompletedProcess` | runs and **waits**; the list is the argument list (no shell, no quoting problems) |
| `capture_output=True, text=True` | | `r.stdout` and `r.stderr` become strings instead of going to the screen |
| `r.returncode` | int | exit status, 0 = success |
| `check=True` | | raises `CalledProcessError` on a non-zero status |
| `subprocess.Popen([...])` | process object | starts and **does not wait**; `p.wait()` later; use to run several in parallel |
| `os.system("cmd")` | wait status | one line, goes through the shell; status is `exit code * 256` on Linux |
| `os.waitstatus_to_exitcode(s)` | int | turns that status back into the exit code |

File `13_subprocess.py3`:

```python
r = subprocess.run(["echo", "zdravo"], capture_output=True, text=True)
print(repr(r.stdout), r.returncode)               # stdout is text because of text=True
try:
    subprocess.run(["false"], check=True)         # ...unless check=True
except subprocess.CalledProcessError as e:
    print("check=True raised, code", e.returncode)

procs = [subprocess.Popen(["sleep", "1"]) for _ in range(3)]   # all three start at once
for p in procs:
    p.wait()                                      # now wait for each
```

```
'zdravo\n' 0
2 ls: cannot access '/nema/takvo/nesto': No such file or directory
check=True raised, code 1
program not found: FileNotFoundError
3 x sleep 1 took 1 s (parallel)
768 3
```

Three `sleep 1` finish in 1 second, not 3, because `Popen` does not wait. `os.system("exit 3")` returned 768, which is `3 * 256`.

### Exit codes between shell and Python

Chain: `Z1.sh` runs `./Z2.py3`; Python prints a result and exits with a status; the shell reads the result with `$( )` and the status with `$?` or `if`. Example from 21.11.2024, in `shell.md` section 16 recipe R6: `n=$(./Z2.py3 all.crypto)`. File `14_exit_status.py3` writes a tiny checker script and calls it from `bash`:

```
polna.txt: output=3 status=0
prazna.txt: output=0 status=1
Upotreba                      <- written to stderr, not captured by $( )
no args: status=2
if sees status 0 as true
```

In short: `print` gives the shell text to capture, `sys.exit(0)` (or just reaching the end) gives status 0, `sys.exit(1)` gives a failure, and an uncaught exception also gives status 1 plus a traceback on stderr.

Beyond this course: `argparse` for flag parsing.

---

## 8. Exam recipes

Every past Python task with the data structure and the loop that solves it. Open the named file for the full solution, and `examples/python/NN_*.py3` for a runnable cleaned-up version.

### R1. 2019 `exam_date_1`: dates first in the line (example `17_recipe_dates_first.py3`)

Task: „… во излезната датотека да ја запише линијата, така што доколку во линијата текст се наоѓа датум, датумот да дојде прв во линијата, па потоа, без менување на редоследот, да дојде и останатиот текст од линијата.“ More than one date: one output line per date. Output file is the input name plus `_out`.

1. Check `len(sys.argv)`; open the input, open `f"{name}_out"` for writing, both in one `with`.
2. For each line: `words = line.split()`; `dates` = distinct words that pass `valid_date` (section 5.1); `rest` = the words that are not dates.
3. For each date write `" ".join([date] + rest) + "\n"`.
4. Optional second argument: if it is a valid date, also print on screen the input lines that contain it.

```python
for line in src:
    words = line.split()
    dates = []
    for w in words:
        if valid(w) and w not in dates:        # distinct valid dates, in order
            dates.append(w)
    rest = [w for w in words if not valid(w)]  # the other words, order unchanged
    for d in dates:
        dst.write(" ".join([d] + rest) + "\n")
```

```
03-05-2020 rodenden vo 31-04-2021 i kraj
29-02-2024 rodenden vo 31-04-2021 i kraj
```

`31-04-2021` is not a real date (April has 30 days), so it stays in the text.

### R2. 2019 `partial_one`: commands and their arguments

Task: „Скриптата треба да ги најде сите наредби (дефинирани во првата датотека) во втората датотека. … Низата аргументи завршува кога ќе се најде првиот збор различен од цртичка.“ A repeated command is printed once, arguments appended to the first one.

1. Two arguments: the command file and the text file; read both with `f.read().split()`.
2. `rez = {}` command -> list of arguments.
3. `while` loop with an index (section 5.4): on a command, swallow the following `-` words.
4. First occurrence: store; later occurrence: append its arguments.
5. Print `command arg arg ...` per entry.

The task text for repeated commands („… започнуваат со „-е“ аргумент“) is ambiguous. Example 08 reads it as "append `-e` and then the new arguments". State your reading in a comment at the top of the script; do not guess silently.

### R3. 2022: search a word, option `l`, `w` or `f` (example `16_recipe_word_flag.py3`)

Task: „… опцијата „l“ значи дека треба да се вратат броевите од линиите во кои е пронајден зборот, „w“ … колку зборови биле пронајдени пред зборот што се бара, и опцијата „f“ … колку пати е пронајден зборот.“ Not found: `l` gives `-1`, `w` and `f` give `0`. Invalid option: stop.

1. `len(sys.argv) != 4`. Validate the option **before** reading the file, exit if it is not `l`, `w`, `f`.
2. One pass with `enumerate(f, start=1)`: keep `linii` (list), `pred` (words before the first hit), `brojac` (hits), `najden` (bool).
3. After the loop, print by option.

```python
for br, line in enumerate(f, start=1):        # line numbers from 1
    for w in line.split():
        if w == zbor:
            najden = True
            brojac += 1
            if br not in linii:
                linii.append(br)
        elif not najden:
            pred += 1                          # words seen before the FIRST match
```

```
l -> 1 2 4          w -> 0          f -> 4          q -> Nevalidna opcija
```

(Input: `a b c / d bb a / x / a a`, searching `a`.) Whether lines count from 0 or 1 is not stated in the task; 1 matches what people expect, say so in a comment.

### R4. 2023: IP addresses, flag `a`, optional `o <file>` (example `06_ip_generate.py3`)

Task: „Знаменцето може да биде „а“. Може да има и опционално дополнително знаменце „о“, после кое следи име на излезна датотека.“ With a mask on the line: print every IP from the address to the mask. Flag `a`: ignore masks, go up to 255.255.255.255.

1. Arguments: input file, flag; if the flag is `o`, then `len(sys.argv) >= 4` and `argv[3]` is the output file. The flag is `sys.argv[2]`, **not** `sys.argv[1]` (that is the file name).
2. Per line: for each word, `parse(word, ".")` gives the IP, `parse(word, ":")` gives the mask (section 5.2).
3. Generate with `box(ip, mask)` (or `box(ip, [255]*4)` for `a`).
4. Output: `print` to screen, or `out.write(addr + "\n")` when an output file was given.

### R5. 2024: crypto currencies, biggest rise and fall (example `18_recipe_crypto.py3`)

Task: „… за секоја одделна крипто-вредност да отпечати кое ќе биде најголемото зголемување од претходната вредност а кое ќе биде најголемото намалување … и помеѓу кои вредности е истото.“ Symbols BTC, Doge, EOS, ETH, XPM; the word after the symbol is the value with a letter prefix. A currency seen zero or one time: print nothing.

1. `vrednosti = {s: [] for s in simboli}` (symbol -> values in file order).
2. One flat `words = f.read().split()`; `enumerate`; collect (section 5.3).
3. For each symbol with at least 2 values: `for a, b in zip(v, v[1:])` visits neighbouring pairs; keep `(difference, a, b)` for the best rise and the best fall.
4. Print the symbol, both differences and both value pairs.

```
Valutata BTC: najgolem porast 60 (90 -> 150), najgolem pad 30 (150 -> 120)
Valutata ETH: najgolem porast 0, najgolem pad 145 (245 -> 100)
```

(`EOS` appears twice with equal values, so rise and fall are both 0. Doge appeared once: nothing is printed.) The solution in `course/2024/Exams/partial_one/Z2.py3` prints "porast od X i pad od Y" but not the value pairs the task asks for; the example adds them.

### R6. 2025 Lab 2 Z1: swap the words of a file (example `19_recipe_swap_words.py3`)

Task: „… да направи замена на првиот со последниот, вториот со претпоследниот и т.н. Доколку зборовите што се заменуваат се исти … да не се прави замена. … Замената се враќа назад во датотеките.“ At least two arguments, each a file name.

1. `len(sys.argv) < 3`: error and exit.
2. For each file name in `sys.argv[1:]`: `try:` read, swap, write back; `except FileNotFoundError:` print a message and go on to the next file.
3. Swap only the first half of the positions: `for i in range(n // 2)`. Swapping all positions undoes itself.
4. Python swaps without a temporary: `zb[i], zb[n-1-i] = zb[n-1-i], zb[i]`.

```
Succes in edit: a.txt
Ne go najdovme fajlot: nema.txt
pet cetiri tri dva eden
```

The "do nothing if the words are equal" rule needs no code: swapping two equal words changes nothing. The course solution writes the words back joined with spaces, so the original line breaks are lost (`a b c / d e` becomes `e d c b a` on one line); the task does not forbid it, but mention it if the exam says "keep the lines".

### R7. 2025 Lab 2 Z2: dates in an `ls -al` file (example `09_ls_dates.py3`)

Task: „Датотеката го чува излезот од ls –al … „-d“ … различните датуми … „-f“ … за секој датум да се отпечати колку датотеки или директориуми биле креирани/модифицирани.“

1. Exactly 2 arguments: file and flag; the file must exist (`os.path.isfile`).
2. Skip lines with fewer than 9 words. Date key: day and month, `f"{f[6]} {f[5]}"`.
3. `-d`: print each key once (a `set`, or the keys of a dict). `-f`: key and count (`defaultdict(int)`).
4. Any other flag: error message, `sys.exit(1)`.

### R8. 2026 Lab 2 Z1: web access log (files `Z1.py3`, `Z1_S.py3`, example `10_access_log.py3`)

Task: „… За секој различен статус код да отпечати колку барања имало со тој статус и колку бајти … трите IP адреси со најмногу барања … сомнителните IP адреси: … во една иста минута направиле повеќе од N барања.“ `-b` writes `blocklist.txt`.

1. Arguments: file, then any mix of a number `N` (default 10) and `-b`, in a loop (section 1).
2. Three dicts, filled in one pass over the valid lines: `status[code] = [requests, bytes]`, `per_ip[ip] = requests`, `per_minute[ip][minute] = requests`.
3. Print status codes with `sorted(status)`; top 3 with a stable sort (section 4); suspicious = every `(ip, minute)` with count `> N`.
4. With `-b`: write each suspicious IP **once**, in order of first appearance.
5. Output formats are fixed by the published files in `course/2026/Labs/Lab2/portal/`: copy the strings exactly (`Status 200: 6 baranja, 12973 bajti`).

---

## 9. Pitfalls

Format: symptom -> cause -> fix. Items 1 to 4 are from the repo's own solutions; run the file named to see each one fail. File `15_pitfalls.py3` demonstrates most of the rest.

1. **The script always prints the usage message, even with the right arguments.**
   Cause: `course/2022/Exams/partial_one/Z2.py3` line 6 reads `if sys.argv!=4:`. `sys.argv` is a list, and a list is never equal to the number 4, so the condition is always true. Fix: `if len(sys.argv) != 4:`.

2. **`IndexError: list index out of range` on the first date, `NameError: name 'ip' is not defined` on the first IP.**
   Cause: `course/2019/Exams/exam_date_1/Z2.py3` line 22 tests `len(varr[3])` after checking that `varr` has 3 pieces (valid indexes are 0, 1, 2; it meant `varr[2]`). `course/2023/Exams/partial_one/Z2.py3` line 35 reads the variable `ip`, but the flag set earlier is called `ip_flag`. Fix: a list with 3 pieces has indexes 0, 1, 2 (the last is `x[-1]`); give each flag one name and use only that name.

3. **A branch that can never run.**
   Cause: in the 2023 solution `if sys.argv[1]=='a':` (line 46) compares the **file name** with `'a'`; the flag is `sys.argv[2]`. Fix: draw the argument list on paper for one sample command line before coding: `prva.py3 ip.txt a` is `[prva.py3, ip.txt, a]`.

4. **The shell script thinks the Python script succeeded.**
   Cause: `course/2024/Exams/partial_one/Z2.py3` line 8 calls `sys.exit()` with no number for the missing-argument error; that exits with 0, "success". Fix: `sys.exit(1)` for every error path (sections 1 and 7).

5. **`ValueError: invalid literal for int() with base 10: 'abc'`.**
   Cause: `int()` on text that is not a number, typically a word of the file that happens to contain `-` or `.`. Fix: `x.isdigit()` first, or `try: ... except ValueError:` (section 3).

6. **Off by one in `argv`.** Symptom: the file name is treated as the flag, or the last argument is missing.
   Cause: `argv[0]` is the script. Fix: `len(sys.argv) != 4` means three real arguments; the first real one is `sys.argv[1]`.

7. **Every line is followed by an empty line.**
   Cause: `readlines()` and `for line in f` keep the `"\n"`, and `print` adds another. Fix: `print(line.rstrip("\n"))`, or `print(line, end="")`. Many of the professor's expected-output files have these blank lines exactly because the solution did not strip, so compare with the published file before "fixing" it.

   ```
   a
   <blank>
   b
   <blank>
   ```

8. **The second loop over the same file does nothing.**
   Cause: a file is read once; after `read()` or a full loop it is at its end, `f.readlines()` returns `[]` (example: `2 0`). Fix: read once into a list, loop over the list as often as you like.

9. **Items are skipped while removing from a list in a loop.**
   Cause: `for n in nums: if ...: nums.remove(n)` shifts the list while it is walked (example prints `[1, 3, 5]` for 1..6 only by luck of the numbers). Fix: build a new list, `nums = [n for n in nums if n % 2]`, or loop over a copy `for n in nums[:]`.

10. **`"10" == 10` is `False`, `"10" > "9"` is `False`, `"5" < 3` raises `TypeError`.**
    Cause: text and number never compare equal, and two texts compare character by character. Fix: convert with `int()` before comparing numbers. This also explains the `-f` order in section 5.5.

11. **The last word of a line has a strange invisible character, comparisons fail.**
    Cause: input files written on Windows end lines with `\r\n`. `split()` removes the `\r`, but `rstrip("\n")` leaves it (example: `'a b\r'`). Fix: `line.strip()` or `line.split()`; `dos2unix file` cleans the file. The repo forces LF through `.gitattributes`.

12. **`if x is "abc"` sometimes true, sometimes false (and a SyntaxWarning).**
    Cause: `is` asks "the same object?", `==` asks "the same value?". Two equal lists are `==` but not `is`. Fix: use `==` for values; `is` only for `None` (`x is None`).

13. **Output file is empty, cut off, or got emptied by accident.**
    Cause: a file opened with plain `open(...)` and never `close()`d keeps data in a buffer until it is closed; and `open(name, "w")` empties the file at once, even if you meant to read it first. Fix: always `with open(...) as f:`; read first, close, then write (2025 Lab 2 Z1 does exactly this), or write to `name + "_out"`.

14. **`IndentationError` or `SyntaxError` on the first run.**
    Cause: mixing tabs and spaces (the 2022 and 2023 files use tabs, the 2024 to 2026 ones four spaces), `i++` (does not exist in Python, use `i += 1`), missing `:` after `if`/`else`/`while`, `os.path.isfile[3]` with square brackets. `course/2025/Exercises/python/Vezba1.py3` and `Vezba2.py3` are unfinished exercise sheets with exactly these errors: a good place to practise reading a traceback. Fix: `python3 -m py_compile file.py3` checks syntax without running.

---

## 10. Quick reference

| Task | Code |
|---|---|
| number of real arguments | `len(sys.argv) - 1` |
| error and stop | `print("greska", file=sys.stderr); sys.exit(1)` |
| read whole file as words | `with open(p) as f: words = f.read().split()` |
| loop lines with number | `for br, line in enumerate(f, start=1):` |
| line without newline | `line.rstrip("\n")` |
| output file `<name>_out` | `open(f"{name}_out", "w")` |
| file exists / is a regular file | `os.path.exists(p)` / `os.path.isfile(p)` |
| missing file does not crash | `try: open(p) ... except FileNotFoundError:` |
| split a date or IP | `s.split("-")` / `s.split(".")` |
| is it a number | `s.isdigit()` (no sign, no point) |
| text to number safely | `try: int(s) except ValueError:` |
| word starts with dash | `w.startswith("-")` |
| cut the first letter | `w[1:]` |
| counter dict | `d[k] = d.get(k, 0) + 1` |
| dict of lists | `d.setdefault(k, []).append(x)` |
| distinct, keep order | `list(dict.fromkeys(items))` |
| top 3 by count, stable | `sorted(d.items(), key=lambda p: p[1], reverse=True)[:3]` |
| neighbouring pairs | `for a, b in zip(v, v[1:]):` |
| biggest by key | `max(items, key=lambda p: p[1])` |
| swap two items | `l[i], l[j] = l[j], l[i]` |
| leap year | `y % 4 == 0 and (y % 100 != 0 or y % 400 == 0)` |
| regex match with check | `m = pat.match(s)` then `if m:` |
| run a program, capture | `subprocess.run([...], capture_output=True, text=True)` |
| parallel programs | `Popen([...])` for each, then `p.wait()` |
| exit status to the shell | `sys.exit(n)`; the shell reads `$?` |
| run without chmod / with | `python3 Z2.py3 a b` / `./Z2.py3 a b` |
| syntax check only | `python3 -m py_compile Z2.py3` |

The examples are numbered in the order of the sections: `01` to `04` sections 1 to 4, `05` to `10` section 5, `11` and `12` section 6, `13` and `14` section 7, `15` section 9, `16` to `19` the recipes of section 8.
