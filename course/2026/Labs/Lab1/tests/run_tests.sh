#!/bin/bash
# Test runner: gradi privremeno drvo, pusta skripti, proveruva rezultati
here="$(cd "$(dirname "$0")" && pwd)"
root="$(cd "$here/.." && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
pass=0; fail=0

check() {  # check "opis" "dobieno" "ocekuvano"
    if [ "$2" == "$3" ]; then echo "PASS: $1"; pass=$((pass+1))
    else echo "FAIL: $1"; echo "  dobieno : [$2]"; echo "  ocekuvano: [$3]"; fail=$((fail+1)); fi
}

for s in Doma_Z1 Doma_Z2 Doma_Z3 Z1 Z2 Z3; do
    bash -n "$root/$s.sh" && echo "PASS: bash -n $s.sh" && pass=$((pass+1)) || { echo "FAIL: bash -n $s.sh"; fail=$((fail+1)); }
done

# ---------- Doma_Z2 ----------
mkdir "$tmp/c" && cd "$tmp/c"
touch a.c b.c notc.txt c.h x.cc
echo "== Doma_Z2"; out="$(bash "$root/Doma_Z2.sh")"; echo "$out"
check "Doma_Z2 samo .c" "$out" "$(printf 'a.c\nb.c')"

# ---------- Doma_Z3 ----------
mkdir "$tmp/s" && cd "$tmp/s"; cp "$here/vezba12.txt" .
echo "== Doma_Z3"; out="$(bash "$root/Doma_Z3.sh")"; echo "$out"
exp="Important e-mail addresses
Official contact email: ?@feit.ukim.edu.mk
Dean's Office contact email: ?@feit.ukim.edu.mk
Dean's Office Assistant: ?@feit.ukim.edu.mk
Public Information Coordinator: ?@feit.ukim.edu.mk
Library: ?@feit.ukim.edu.mk
Web Administrator: ?@feit.ukim.edu.mk"
check "Doma_Z3 validni cenzurirani" "$(echo "$out" | head -7)" "$exp"
check "Doma_Z3 nevalidni neizmeneti" "$(echo "$out" | tail -n +8)" "$(tail -n +8 vezba12.txt)"
bash "$root/Doma_Z3.sh" nema.txt; check "Doma_Z3 nema datoteka exit 1" "$?" "1"

# ---------- Z1 ----------
mkdir -p "$tmp/z1/sub" && cd "$tmp/z1"
touch here.txt sub/deep.txt
echo "== Z1 (2 od 3)"; out="$(bash "$root/Z1.sh" here.txt deep.txt missing.txt)"; echo "$out"
check "Z1 procent" "$(echo "$out" | tail -1)" "Procent na najdeni datoteki: 66%"
check "Z1 deep najdena" "$(echo "$out" | grep -c 'deep.txt - najdena')" "1"
check "Z1 missing ne e najdena" "$(echo "$out" | grep -c 'missing.txt - ne e najdena')" "1"
echo "== Z1 (bez argumenti)"; bash "$root/Z1.sh"; check "Z1 bez argumenti exit 1" "$?" "1"

# ---------- Z2 scenario A: i vo cwd i vo sub ----------
mkdir -p "$tmp/z2a/sub/dir" && cd "$tmp/z2a"
echo "cwd copy" > naredbi.txt; echo "sub copy" > sub/dir/naredbi.txt
echo "== Z2 A"; out="$(bash "$root/Z2.sh")"; echo "$out"
check "Z2 A cwd izbrisana" "$([ -e naredbi.txt ] && echo da || echo ne)" "ne"
check "Z2 A pecati sub" "$(echo "$out" | tail -1)" "sub copy"
# ---------- Z2 scenario B: samo vo sub ----------
mkdir -p "$tmp/z2b/sub" && cd "$tmp/z2b"
echo "only sub" > sub/naredbi.txt
echo "== Z2 B"; out="$(bash "$root/Z2.sh")"; echo "$out"
check "Z2 B sub ostanuva" "$([ -e sub/naredbi.txt ] && echo da || echo ne)" "da"
check "Z2 B pecati" "$out" "only sub"
# scenario C: samo cwd; D: ime kako argument; E: ne postoi
mkdir "$tmp/z2c" && cd "$tmp/z2c"; echo "only cwd" > naredbi.txt
out="$(bash "$root/Z2.sh")"; check "Z2 C samo cwd ostanuva i se pecati" "$out|$([ -e naredbi.txt ] && echo da)" "only cwd|da"
mkdir -p "$tmp/z2d/sub" && cd "$tmp/z2d"; echo top > my.txt; echo low > sub/my.txt
out="$(bash "$root/Z2.sh" my.txt)"; check "Z2 D arg: pecati sub" "$(echo "$out" | tail -1)" "low"; check "Z2 D arg: cwd izbrisana" "$([ -e my.txt ] && echo da || echo ne)" "ne"
bash "$root/Z2.sh" nema.txt; check "Z2 E ne postoi exit 1" "$?" "1"

# ---------- Z3 ----------
mkdir -p "$tmp/z3/a/b" "$tmp/z3/c" && cd "$tmp/z3"
printf 'x=1\npassword = "abc"\ny=2\n' > a/pw1.py
printf 'import os\nmy_password="z"\n' > a/b/pw2.py3
printf 'print(1)\n' > a/clean.py
printf 'nothing\n' > c/clean2.py3
printf 'password=top\n' > here.py
printf 'password in txt\n' > c/skip.txt
echo "== Z3"; out="$(bash "$root/Z3.sh")"; echo "$out"
check "Z3 kopirani" "$(ls | tr '\n' ' ')" "a c here.py pw1.py pw2.py3 "
check "Z3 pecatenje" "$(echo "$out" | sort)" "$(printf './pw1.py:2:password = "abc"\n./pw2.py3:2:my_password="z"')"
check "Z3 cwd skrip ne e kopiran/pecaten" "$(echo "$out" | grep -c here.py)" "0"
mkdir "$tmp/z3n" && cd "$tmp/z3n"; echo hi > q.py
out="$(bash "$root/Z3.sh")"; check "Z3 nema kopirani" "$out" "Nema kopirani datoteki"

echo; echo "REZULTAT: $pass PASS, $fail FAIL"
[ "$fail" -eq 0 ]
