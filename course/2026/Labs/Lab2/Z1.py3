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
# ПОЕДНОСТАВЕНА ВЕРЗИЈА: без регуларни изрази, само readlines() и split(). Истиот излез како Z1_S.py3 (верзијата со регуларен израз).
# Една валидна линија по split() има точно 10 зборови:
#   0: IP    1: -    2: -    3: [03/Nov/2025:15:17:02    4: +0100]
#   5: "GET  6: /index.html  7: HTTP/1.1"  8: 200  9: 1043
# Линија што нема 10 зборови, или кај која статусот и бајтите не се броеви, не го следи форматот и се прескокнува.
# Минутата е зборот 3 без „[“ на почетокот и без „:СС“ на крајот: 03/Nov/2025:15:17
import sys

if len(sys.argv) < 2:
    print("Greshka: ne e navedena vlezna datoteka", file=sys.stderr)
    sys.exit(1)

ime = sys.argv[1]
N = 10
zapishi_blok = False
for arg in sys.argv[2:]:
    if arg == "-b":
        zapishi_blok = True
    elif arg.isdigit():
        N = int(arg)
    else:
        print("Greshka: N mora da bide cel broj", file=sys.stderr)
        sys.exit(1)

try:
    f = open(ime, "r")
except OSError:
    print("Greshka: ne mozham da ja otvoram datotekata", ime, file=sys.stderr)
    sys.exit(1)
linii = f.readlines()
f.close()

statusi = {}     # status -> [broj baranja, bajti]
broj_ip = {}     # ip -> broj baranja
po_minuta = {}   # ip -> {minuta: broj baranja}

for linija in linii:
    zborovi = linija.split()
    if len(zborovi) != 10:
        continue
    ip = zborovi[0]
    vreme = zborovi[3]        # [03/Nov/2025:15:17:02
    status = zborovi[8]
    bajti = zborovi[9]
    if not vreme.startswith("[") or not status.isdigit() or not bajti.isdigit():
        continue
    minuta = vreme[1:-3]      # 03/Nov/2025:15:17
    status = int(status)
    bajti = int(bajti)

    if status not in statusi:
        statusi[status] = [0, 0]
    statusi[status][0] += 1
    statusi[status][1] += bajti

    if ip not in broj_ip:
        broj_ip[ip] = 0
    broj_ip[ip] += 1

    if ip not in po_minuta:
        po_minuta[ip] = {}
    if minuta not in po_minuta[ip]:
        po_minuta[ip][minuta] = 0
    po_minuta[ip][minuta] += 1

# 1. statistika po status, rastecki, pa vkupno
vkupno_baranja = 0
vkupno_bajti = 0
for status in sorted(statusi):
    broj, bajti = statusi[status]
    print(f"Status {status}: {broj} baranja, {bajti} bajti")
    vkupno_baranja += broj
    vkupno_bajti += bajti
print(f"Vkupno: {vkupno_baranja} baranja, {vkupno_bajti} bajti")

# 2. trite najaktivni IP adresi (sorted e stabilen: pri ist broj ostanuva redosledot na pojavuvanje)
print("Najaktivni IP adresi:")
podredeni = sorted(broj_ip.items(), key=lambda par: par[1], reverse=True)
for ip, broj in podredeni[:3]:
    print(f"{ip} {broj}")

# 3. somnitelni: povekje od N baranja vo ista minuta (recnicite go cuvaat redosledot na vnesuvanje)
somnitelni = []   # lista od (ip, minuta, broj)
for ip in po_minuta:
    for minuta in po_minuta[ip]:
        if po_minuta[ip][minuta] > N:
            somnitelni.append((ip, minuta, po_minuta[ip][minuta]))

if len(somnitelni) == 0:
    print(f"Somnitelni IP adresi (povekje od {N} baranja vo edna minuta): nema")
else:
    print(f"Somnitelni IP adresi (povekje od {N} baranja vo edna minuta):")
    for ip, minuta, broj in somnitelni:
        print(f"{ip} ({broj} baranja vo {minuta})")

# -b: sekoja somnitelna IP ednash vo blocklist.txt
if zapishi_blok:
    f = open("blocklist.txt", "w")
    zapishani = []
    for ip, minuta, broj in somnitelni:
        if ip not in zapishani:
            zapishani.append(ip)
            f.write(ip + "\n")
    f.close()
