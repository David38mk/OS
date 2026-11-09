# python.md §6 — re: match/search/findall/sub/compile, groups, raw strings
import re
line = '10.0.0.7 - - [03/Nov/2025:15:17:02 +0100] "GET /index.html HTTP/1.1" 200 1043'
print(re.match(r"\d+\.\d+", line).group())          # match: must start at position 0
print(re.match(r"GET", line))                       # None, "GET" is not at the start
print(re.search(r"GET", line).start())              # search: anywhere in the string
print(re.findall(r"\d{2}-\d{2}-\d{4}", "a 01-02-2024 b 31-12-1999"))   # list of all matches
print(re.sub(r"\s+", " ", "a   b \t c"))            # replace every match
m = re.search(r"\[(\d{2})/(\w{3})/(\d{4})", line)   # () = capture group
print(m.group(0), m.groups(), m.group(2))
print(re.findall(r"(\w+)=(\d+)", "a=1 b=22"))       # with groups, findall gives tuples
pat = re.compile(r"^(\S+) .* (\d{3}) (\d+)$")       # compile once, reuse in a loop
m = pat.match(line)
print(m.group(1), int(m.group(2)), int(m.group(3)))
print(pat.match("bad line"))                        # None: always test before .group()
print(r"\d" == "\\d", len("\n"), len(r"\n"))        # raw string: backslash stays a backslash
