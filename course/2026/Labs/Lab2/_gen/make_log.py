#!/usr/bin/env python3
# Builds the portal package for Lab 2 (2026/27) task 1 into ../portal/:
#   access.log            the 11 sample lines from the lab text
#   access_golem.log      a larger, deterministic log for real testing
#   izlez*.txt, blocklist*.txt   expected outputs, produced by z1_ref.py3
# Run from anywhere: python3 make_log.py
import os
import random
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
PORTAL = os.path.join(HERE, "..", "portal")
REF = os.path.join(HERE, "..", "Z1.py3")
DAYS = ["03/Nov/2025", "04/Nov/2025", "05/Nov/2025"]
TZ = "+0100"

SAMPLE = """\
10.0.0.7 - - [03/Nov/2025:15:17:02 +0100] "GET /index.html HTTP/1.1" 200 1043
10.0.0.7 - - [03/Nov/2025:15:17:05 +0100] "GET /login HTTP/1.1" 200 512
192.168.1.20 - - [03/Nov/2025:15:17:09 +0100] "POST /login HTTP/1.1" 401 231
192.168.1.20 - - [03/Nov/2025:15:17:11 +0100] "POST /login HTTP/1.1" 401 231
192.168.1.20 - - [03/Nov/2025:15:17:14 +0100] "POST /login HTTP/1.1" 401 231
192.168.1.20 - - [03/Nov/2025:15:17:16 +0100] "POST /login HTTP/1.1" 401 231
10.0.0.9 - - [03/Nov/2025:15:18:01 +0100] "GET /slike/logo.png HTTP/1.1" 200 8820
10.0.0.7 - - [03/Nov/2025:15:18:30 +0100] "GET /nema HTTP/1.1" 404 196
192.168.1.20 - - [03/Nov/2025:15:18:40 +0100] "POST /login HTTP/1.1" 200 512
10.0.0.9 - - [03/Nov/2025:15:19:00 +0100] "GET /index.html HTTP/1.1" 200 1043
10.0.0.11 - - [03/Nov/2025:15:19:03 +0100] "GET /index.html HTTP/1.1" 200 1043
"""

# (method, path, bytes) per status; normal traffic picks a status by weight
REQ = {
    200: [("GET", "/index.html", 1043), ("GET", "/login", 512), ("GET", "/slike/logo.png", 8820),
          ("GET", "/css/stil.css", 2310), ("GET", "/js/app.js", 15320), ("GET", "/kontakt.html", 1780),
          ("GET", "/api/ceni", 640), ("POST", "/login", 512)],
    301: [("GET", "/stara", 231)],
    304: [("GET", "/slike/logo.png", 0), ("GET", "/css/stil.css", 0)],
    401: [("POST", "/login", 231)],
    403: [("GET", "/admin", 199)],
    404: [("GET", "/nema", 196), ("GET", "/favicon.ico", 196), ("GET", "/admin.php", 196)],
    500: [("GET", "/api/ceni", 312)],
}
WEIGHTS = {200: 75, 304: 8, 404: 8, 401: 4, 301: 2, 403: 2, 500: 1}

# normal IPs with their total request count over the three days (all distinct,
# and distinct from the burst sizes, so "top 3" has no ties)
NORMAL = {"10.0.0.7": 42, "192.168.1.20": 36, "10.0.0.9": 24, "89.205.3.77": 22,
          "77.28.100.12": 19, "10.0.0.11": 17, "10.0.0.23": 15, "192.168.1.31": 13,
          "146.255.80.2": 9, "192.168.1.44": 7}

# bursts: ip, day, minute, count, request. Order of appearance = descending count
# = ascending IP string, so three natural output orders agree.
BURSTS = [
    ("172.16.0.9",    0, 15 * 60 + 17, 25, ("POST", "/login", 401, 231)),   # > 10 and > 3
    ("185.220.101.4", 1,  2 * 60 + 31, 14, ("GET", "/admin", 403, 199)),    # > 10 and > 3
    ("192.168.1.77",  1, 11 * 60 + 5,  10, ("GET", "/api/ceni", 200, 640)),  # exactly 10: only > 3
    ("203.0.113.8",   2,  9 * 60 + 40,  6, ("GET", "/nema", 404, 196)),     # only > 3
    ("45.33.32.156",  2, 17 * 60 + 22,  4, ("POST", "/login", 401, 231)),   # exactly 4: only > 3
]

MALFORMED = [
    (40,  '10.0.0.7 - - [03/Nov/2025:15:20:00 +0100] "GET /x HTTP/1.1"'),          # no status/bytes
    (90,  ''),                                                                       # empty line
    (130, 'ova ne e validna linija od log'),                                         # garbage
    (170, '192.168.1.5 - - [04/Nov/2025:09:00:00 +0100] "GET /a HTTP/1.1" 200 abc'),  # bytes not a number
    (210, '- - [04/Nov/2025:09:00:01 +0100] "GET /a HTTP/1.1" 200 10'),             # no IP
    (250, '[05/Nov/2025:10:00:00 +0100]'),                                          # timestamp only
]

rng = random.Random(2026)
entries = []  # (day, minute, second, seq, line)
seq = 0


def add(day, minute, second, ip, method, path, code, nbytes):
    global seq
    seq += 1
    ts = f"{DAYS[day]}:{minute // 60:02d}:{minute % 60:02d}:{second:02d} {TZ}"
    entries.append((day, minute, second, seq,
                    f'{ip} - - [{ts}] "{method} {path} HTTP/1.1" {code} {nbytes}'))


def random_request():
    code = rng.choices(list(WEIGHTS), weights=list(WEIGHTS.values()))[0]
    method, path, nbytes = rng.choice(REQ[code])
    return method, path, code, nbytes


# 1. the first seven lines introduce every status code in ascending order
intro = [200, 301, 304, 401, 403, 404, 500]
for i, code in enumerate(intro):
    method, path, nbytes = REQ[code][0]
    add(0, 8 * 60 + i, rng.randrange(60), "10.0.0.7", method, path, code, nbytes)

# 2. explicit boundary cases inside normal traffic
#    10.0.0.23: exactly 3 requests in one minute (not suspicious for N=3)
for s in (5, 20, 41):
    add(0, 12 * 60 + 10, s, "10.0.0.23", *random_request())
#    89.205.3.77: 3 + 3 requests across a minute boundary (a sliding window would
#    see 6, the task's "same minute" rule sees 3 and 3)
for s in (57, 58, 59):
    add(1, 16 * 60 + 59, s, "89.205.3.77", *random_request())
for s in (0, 1, 2):
    add(1, 17 * 60 + 0, s, "89.205.3.77", *random_request())

used_explicit = {"10.0.0.7": 7, "10.0.0.23": 3, "89.205.3.77": 6}
blocked_minutes = {("10.0.0.23", 0): {12 * 60 + 10},
                   ("89.205.3.77", 1): {16 * 60 + 59, 17 * 60 + 0}}

# 3. normal traffic: one request per distinct minute, 08:20-23:00
for ip, total in NORMAL.items():
    remaining = total - used_explicit.get(ip, 0)
    per_day = [remaining // 3] * 3
    for d in range(remaining % 3):
        per_day[d] += 1
    for day, k in enumerate(per_day):
        pool = [m for m in range(8 * 60 + 20, 23 * 60) if m not in blocked_minutes.get((ip, day), set())]
        for minute in rng.sample(pool, k):
            add(day, minute, rng.randrange(60), ip, *random_request())

# 4. bursts
for ip, day, minute, count, (method, path, code, nbytes) in BURSTS:
    for second in sorted(rng.sample(range(60), count)):
        add(day, minute, second, ip, method, path, code, nbytes)

entries.sort()
lines = [e[4] for e in entries]

# ---- sanity checks on the generated data ----
from collections import Counter
per_ip = Counter()
per_min = Counter()
for day, minute, second, _, line in entries:
    ip = line.split()[0]
    per_ip[ip] += 1
    per_min[(ip, day, minute)] += 1
burst_ips = {b[0] for b in BURSTS}
for (ip, day, minute), c in per_min.items():
    if ip not in burst_ips:
        assert c <= 3, (ip, day, minute, c)
assert {ip for (ip, _, _), c in per_min.items() if c > 10} == {"172.16.0.9", "185.220.101.4"}
assert {ip for (ip, _, _), c in per_min.items() if c > 3} == burst_ips
for ip in burst_ips:
    assert sum(1 for (i, _, _), c in per_min.items() if i == ip and c > 3) == 1
top = per_ip.most_common(4)
assert len({c for _, c in top}) == 4, top
for ip, total in NORMAL.items():
    assert per_ip[ip] == total, (ip, per_ip[ip], total)

# 5. insert malformed lines at fixed positions
for pos, text in sorted(MALFORMED, reverse=True):
    lines.insert(pos, text)

os.makedirs(PORTAL, exist_ok=True)


def write_lf(name, text):
    with open(os.path.join(PORTAL, name), "w", encoding="utf-8", newline="\n") as f:
        f.write(text)


write_lf("access.log", SAMPLE)
write_lf("access_golem.log", "\n".join(lines) + "\n")


def run_ref(args, out_name, blocklist_name=None):
    """Run the reference in the portal folder; normalise CRLF so the files are
    identical to what the lab PCs would produce."""
    r = subprocess.run([sys.executable, "-I", REF] + args, cwd=PORTAL, capture_output=True)
    assert r.returncode == 0, r.stderr
    with open(os.path.join(PORTAL, out_name), "wb") as f:
        f.write(r.stdout.replace(b"\r\n", b"\n"))
    if blocklist_name:
        p = os.path.join(PORTAL, "blocklist.txt")
        data = open(p, "rb").read().replace(b"\r\n", b"\n")
        os.remove(p)
        with open(os.path.join(PORTAL, blocklist_name), "wb") as f:
            f.write(data)


# the reference always writes blocklist.txt, so the sample run (whose target
# is that very name) must come last
run_ref(["access_golem.log"], "izlez_golem1.txt")
run_ref(["access_golem.log", "3"], "izlez_golem2.txt")
run_ref(["access_golem.log", "3", "-b"], "tmp.out", "blocklist_golem.txt")
run_ref(["access.log"], "izlez1.txt")
run_ref(["access.log", "3"], "izlez2.txt")
run_ref(["access.log", "3", "-b"], "tmp.out", "blocklist.txt")
os.remove(os.path.join(PORTAL, "tmp.out"))

print(f"access_golem.log: {len(lines)} lines ({len(entries)} valid, {len(MALFORMED)} malformed)")
print("top IPs:", per_ip.most_common(4))
