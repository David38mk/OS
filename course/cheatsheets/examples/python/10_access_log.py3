# python.md §5 — access log: a valid line has exactly 10 words after split(); count per status
import io
log = io.StringIO("""10.0.0.7 - - [03/Nov/2025:15:17:02 +0100] "GET /index.html HTTP/1.1" 200 1043
10.0.0.7 - - [03/Nov/2025:15:17:05 +0100] "GET /login HTTP/1.1" 200 512
bad line
192.168.1.20 - - [03/Nov/2025:15:17:09 +0100] "POST /login HTTP/1.1" 401 231
192.168.1.20 - - [03/Nov/2025:15:17:09 +0100] "POST /login HTTP/1.1" 4x1 231
""")
status = {}                                   # code -> [requests, bytes]
for line in log:
    w = line.split()
    # 0 ip  1 -  2 -  3 [date:time  4 +0100]  5 "GET  6 /path  7 HTTP/1.1"  8 status  9 bytes
    if len(w) != 10 or not w[8].isdigit() or not w[9].isdigit():
        continue                              # does not follow the format: skip
    minuta = w[3][1:-3]                       # "[03/Nov/2025:15:17:02" -> "03/Nov/2025:15:17"
    st = status.setdefault(int(w[8]), [0, 0])
    st[0] += 1
    st[1] += int(w[9])
for code in sorted(status):
    print(f"Status {code}: {status[code][0]} baranja, {status[code][1]} bajti")
print(minuta)
