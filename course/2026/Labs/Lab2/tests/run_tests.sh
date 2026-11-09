#!/bin/bash
# Test runner za Lab 2: Z1 (python3), Z2 i Z2_threads (C, so laznen ping)
if [ "$(uname)" != Linux ]; then echo "Pushti vo Linux kutijata: ssh oslab"; exit 1; fi
here="$(cd "$(dirname "$0")" && pwd)"
root="$(cd "$here/.." && pwd)"
portal="$root/portal"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
pass=0; fail=0

check() {  # check "opis" "dobieno" "ocekuvano"
    if [ "$2" == "$3" ]; then echo "PASS: $1"; pass=$((pass+1))
    else echo "FAIL: $1"; echo "  dobieno : [$2]"; echo "  ocekuvano: [$3]"; fail=$((fail+1)); fi
}

# ---------- Z1 (regex) i Z1_simple (readlines/split): ist izlez ----------
sed 's/$/\r/' "$portal/access.log" > "$tmp/crlf.log"
for z in Z1 Z1_simple; do
    echo "== $z"
    s="$root/$z.py3"
    cd "$tmp"
    python3 "$s" 2>"$tmp/err"; rc=$?
    check "$z bez argumenti: exit 1" "$rc" "1"
    check "$z bez argumenti: stderr ne e prazen" "$([ -s "$tmp/err" ] && echo da || echo ne)" "da"
    python3 "$s" nema.log >/dev/null 2>&1; check "$z nepostoecka datoteka: exit 1" "$?" "1"
    python3 "$s" "$portal/access.log" abc >/dev/null 2>&1; check "$z N=abc: exit 1" "$?" "1"

    check "$z access.log" "$(python3 "$s" "$portal/access.log")" "$(cat "$portal/izlez1.txt")"
    check "$z access.log 3" "$(python3 "$s" "$portal/access.log" 3)" "$(cat "$portal/izlez2.txt")"
    mkdir -p "$tmp/b_$z" && cd "$tmp/b_$z"
    python3 "$s" "$portal/access.log" 3 -b >/dev/null
    check "$z access.log 3 -b blocklist" "$(cat blocklist.txt)" "$(cat "$portal/blocklist.txt")"

    check "$z access_golem.log" "$(python3 "$s" "$portal/access_golem.log")" "$(cat "$portal/izlez_golem1.txt")"
    check "$z access_golem.log 3" "$(python3 "$s" "$portal/access_golem.log" 3)" "$(cat "$portal/izlez_golem2.txt")"
    mkdir -p "$tmp/g_$z" && cd "$tmp/g_$z"
    python3 "$s" "$portal/access_golem.log" 3 -b >/dev/null
    check "$z access_golem.log 3 -b blocklist" "$(cat blocklist.txt)" "$(cat "$portal/blocklist_golem.txt")"

    check "$z CRLF datoteka" "$(python3 "$s" "$tmp/crlf.log")" "$(cat "$portal/izlez1.txt")"
done

# ---------- Kompajliranje ----------
echo "== Kompajliranje"
gcc -Wall -Wextra -pthread -o "$tmp/Z2" "$root/Z2.c" 2>"$tmp/w1"; rc=$?
check "Z2.c kompajlira (exit 0)" "$rc" "0"
check "Z2.c bez warnings" "$(cat "$tmp/w1")" ""
gcc -Wall -Wextra -pthread -o "$tmp/Z2t" "$root/Z2_threads.c" 2>"$tmp/w2"; rc=$?
check "Z2_threads.c kompajlira (exit 0)" "$rc" "0"
check "Z2_threads.c bez warnings" "$(cat "$tmp/w2")" ""

# ---------- Laznen ping ----------
mkdir "$tmp/bin"
cat > "$tmp/bin/ping" <<'EOF'
#!/bin/bash
host="${@: -1}"
case "$host" in
  up.test) exit 0;;
  down.test) echo "ping: $host: Name or service not known" >&2; exit 2;;
  slow.test) exec sleep 30;;
  late.test) exec sleep 2;;
  mid.test) exec sleep 1.5;;
  flaky.test) n=$(cat "$FLAKY_FILE" 2>/dev/null || echo 0); n=$((n+1)); echo $n > "$FLAKY_FILE"; [ "$n" -ge 2 ] && exit 0 || exit 2;;
  *) exit 1;;
esac
EOF
chmod +x "$tmp/bin/ping"
export PATH="$tmp/bin:$PATH"
export FLAKY_FILE="$tmp/flaky.cnt"
cd "$tmp"
strip() { sed 's/^Vnesi timeout vo sekundi: //'; }

exp3="$(printf 'up.test UP (0 s)\ndown.test DOWN (0 s)\nslow.test TIMEOUT (2 s)\nZavrshuva process 0')"

# ---------- Z2 ----------
echo "== Z2"
SECONDS=0
out="$(echo 2 | "$tmp/Z2" up.test down.test slow.test | strip)"
el=$SECONDS
check "Z2 up/down/timeout" "$out" "$exp3"
check "Z2 paralelno (<= 3 s)" "$([ "$el" -le 3 ] && echo ok || echo "sporo: $el s")" "ok"

out="$(echo 2 | "$tmp/Z2" slow.test up.test | strip)"
check "Z2 redosled na argumenti" "$(echo "$out" | sed -n 1p)|$(echo "$out" | sed -n 2p)" "slow.test TIMEOUT (2 s)|up.test UP (0 s)"

out="$(echo 4 | "$tmp/Z2" late.test | strip | head -1)"
if [[ "$out" =~ ^late.test\ UP\ \([23]\ s\)$ ]]; then check "Z2 late.test UP po 2-3 s" "ok" "ok"
else check "Z2 late.test UP po 2-3 s" "$out" "late.test UP (2 s) ili (3 s)"; fi

check "Z2 nema sirotni sleep 30" "$(pgrep -f "sleep 30")" ""

# 1.5 s pod T=2 mora da e UP: so time() (rezolucija 1 s) timeout-ot bi se aktiviral predvreme
out="$(echo 2 | "$tmp/Z2" mid.test | strip | head -1)"
check "Z2 mid.test (1.5 s < T=2) e UP, ne TIMEOUT" "$out" "mid.test UP (1 s)"

"$tmp/Z2" >/dev/null 2>"$tmp/err"; rc=$?
check "Z2 bez argumenti: exit 1" "$rc" "1"
check "Z2 bez argumenti: stderr ne e prazen" "$([ -s "$tmp/err" ] && echo da || echo ne)" "da"

SECONDS=0
out="$(echo 2 | "$tmp/Z2" -r 3 up.test down.test | strip)"
el=$SECONDS
check "Z2 -r 3" "$out" "$(printf 'up.test UP (0 s)\ndown.test DOWN (0 s) posle 3 obidi\nZavrshuva process 0')"
check "Z2 -r 3 vreme (<= 4 s)" "$([ "$el" -le 4 ] && echo ok || echo "sporo: $el s")" "ok"

rm -f "$FLAKY_FILE"
out="$(echo 2 | "$tmp/Z2" -r 3 flaky.test | strip | head -1)"
check "Z2 -r 3 flaky: UP posle 2 obidi" "$out" "flaky.test UP (0 s) posle 2 obidi"

echo 2 | "$tmp/Z2" -r abc h >/dev/null 2>&1; r1=$?
echo 2 | "$tmp/Z2" -r 0 h >/dev/null 2>&1; r2=$?
echo 2 | "$tmp/Z2" -r 2 >/dev/null 2>&1; r3=$?
check "Z2 nevalidno -r (abc, 0, bez host): exit 1" "$r1 $r2 $r3" "1 1 1"

echo 2 | "$tmp/Z2" up.test > "$tmp/o.txt"
check "Z2 prompt samo 1 pat (fflush pred fork)" "$(grep -c 'Vnesi timeout' "$tmp/o.txt")" "1"

echo abc | "$tmp/Z2" up.test >/dev/null 2>&1; r1=$?
echo 0 | "$tmp/Z2" up.test >/dev/null 2>&1; r2=$?
check "Z2 nevalidno T (abc, 0): exit 1" "$r1 $r2" "1 1"

# ---------- Z2_threads ----------
echo "== Z2_threads"
out="$(echo 2 | "$tmp/Z2t" up.test down.test slow.test | strip)"
check "Z2_threads up/down/timeout" "$out" "$exp3"

out="$(echo 2 | "$tmp/Z2t" slow.test up.test | strip)"
check "Z2_threads redosled na argumenti" "$(echo "$out" | sed -n 1p)|$(echo "$out" | sed -n 2p)" "slow.test TIMEOUT (2 s)|up.test UP (0 s)"

out="$(echo 4 | "$tmp/Z2t" late.test | strip | head -1)"
if [[ "$out" =~ ^late.test\ UP\ \([23]\ s\)$ ]]; then check "Z2_threads late.test UP po 2-3 s" "ok" "ok"
else check "Z2_threads late.test UP po 2-3 s" "$out" "late.test UP (2 s) ili (3 s)"; fi

check "Z2_threads nema sirotni sleep 30" "$(pgrep -f "sleep 30")" ""

out="$(echo 2 | "$tmp/Z2t" mid.test | strip | head -1)"
check "Z2_threads mid.test (1.5 s < T=2) e UP, ne TIMEOUT" "$out" "mid.test UP (1 s)"

echo
echo "Vkupno: $pass PASS, $fail FAIL"
[ "$fail" -eq 0 ] || exit 1
