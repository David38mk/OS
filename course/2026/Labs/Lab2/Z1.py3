#!/usr/bin/python3
# Задача 1: Да се напише Python скрипта која како прв аргумент добива име на влезна датотека. Доколку скриптата
# се повика без аргументи, да заврши со порака за грешка. Влезната датотека е access log од веб сервер, во која
# секоја линија го има следниот формат:
#
#   IP - - [ДД/Мес/ГГГГ:ЧЧ:ММ:СС +0100] "МЕТОД /патека HTTP/1.1" СТАТУС БАЈТИ
#
# Скриптата треба:
#   1. За секој различен статус код да отпечати колку барања имало со тој статус и колку бајти се испратени
#      вкупно, а на крај вкупниот број на барања и бајти за целата датотека.
#   2. Да ги отпечати трите IP адреси со најмногу барања, подредени опаѓачки.
#   3. Да ги најде „сомнителните“ IP адреси: тоа се адреси кои во една иста минута направиле повеќе од N
#      барања. N е вториот, опционален аргумент од командна линија; доколку не е наведен, N е 10. За секоја
#      сомнителна адреса се печати колку барања направила и во која минута.
#
# Доколку како трет аргумент се наведе знаменцето `-b`, скриптата дополнително ги запишува сомнителните IP
# адреси во датотека `blocklist.txt`, по една адреса во линија. Линии кои не го следат форматот да се прескокнат.
#
# Конвенции на излезот (текстот не ги определува, ги определуваат објавените датотеки):
#   - статус кодовите растечки
#   - топ 3 IP по број на барања опаѓачки; при изедначување важи редоследот на прво појавување
#   - сомнителните IP по редоследот на прво појавување во логот, по една линија за секој (IP, минута) над N
import re
import sys

if len(sys.argv) < 2:
    print("Greshka: ne e navedena vlezna datoteka", file=sys.stderr)
    sys.exit(1)

fname = sys.argv[1]
N = 10
write_block = False
for arg in sys.argv[2:]:
    if arg == "-b":
        write_block = True
    else:
        try:
            N = int(arg)
        except ValueError:
            print("Greshka: N mora da bide cel broj", file=sys.stderr)
            sys.exit(1)

# IP - - [dd/Mon/yyyy:HH:MM:SS +zzzz] "request" status bytes
pat = re.compile(
    r'^(\S+) \S+ \S+ \[(\d{2}/[A-Za-z]{3}/\d{4}:\d{2}:\d{2}):\d{2} [+-]\d{4}\] '
    r'"[^"]*" (\d{3}) (\d+)$'
)

status = {}   # code -> [requests, bytes]
per_ip = {}   # ip -> requests            (insertion order = first appearance)
per_min = {}  # ip -> {minute: requests}  (insertion order = first appearance)

try:
    f = open(fname, "r")
except OSError:
    print(f"Greshka: ne mozham da ja otvoram datotekata {fname}", file=sys.stderr)
    sys.exit(1)

with f:
    for line in f:
        m = pat.match(line.rstrip("\r\n"))
        if not m:
            continue  # line does not follow the format
        ip, minute, code, nbytes = m.group(1), m.group(2), int(m.group(3)), int(m.group(4))
        st = status.setdefault(code, [0, 0])
        st[0] += 1
        st[1] += nbytes
        per_ip[ip] = per_ip.get(ip, 0) + 1
        mins = per_min.setdefault(ip, {})
        mins[minute] = mins.get(minute, 0) + 1

total_req = sum(v[0] for v in status.values())
total_bytes = sum(v[1] for v in status.values())
for code in sorted(status):
    print(f"Status {code}: {status[code][0]} baranja, {status[code][1]} bajti")
print(f"Vkupno: {total_req} baranja, {total_bytes} bajti")

print("Najaktivni IP adresi:")
for ip, cnt in sorted(per_ip.items(), key=lambda kv: -kv[1])[:3]:
    print(f"{ip} {cnt}")

suspicious = [(ip, minute, cnt)
              for ip, mins in per_min.items()
              for minute, cnt in mins.items() if cnt > N]
if suspicious:
    print(f"Somnitelni IP adresi (povekje od {N} baranja vo edna minuta):")
    for ip, minute, cnt in suspicious:
        print(f"{ip} ({cnt} baranja vo {minute})")
else:
    print(f"Somnitelni IP adresi (povekje od {N} baranja vo edna minuta): nema")

if write_block:
    with open("blocklist.txt", "w") as out:
        for ip in dict.fromkeys(ip for ip, _, _ in suspicious):
            out.write(ip + "\n")
