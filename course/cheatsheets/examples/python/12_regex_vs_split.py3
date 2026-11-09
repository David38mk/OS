# python.md §6 — regex vs split on the same lines: split breaks on a request with spaces, regex does not
import re
lines = ['10.0.0.7 - - [03/Nov/2025:15:17:02 +0100] "GET /index.html HTTP/1.1" 200 1043',
         '10.0.0.8 - - [03/Nov/2025:15:17:03 +0100] "GET /a b.html HTTP/1.1" 200 5']
pat = re.compile(r'^(\S+) \S+ \S+ \[([^\]:]+:\d\d:\d\d):\d\d [+-]\d{4}\] "[^"]*" (\d{3}) (\d+)$')
for line in lines:
    w = line.split()
    by_split = (w[0], w[8], w[9]) if len(w) == 10 else None
    m = pat.match(line)
    by_regex = (m.group(1), m.group(3), m.group(4)) if m else None
    print(len(w), by_split, by_regex)
