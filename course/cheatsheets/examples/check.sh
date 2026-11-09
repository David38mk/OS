#!/bin/bash
# Checks every cheat-sheet example: C files compile warning-free with
# gcc -Wall -Wextra -pthread, driver modules build against the installed kernel
# headers, Python files pass a syntax check. With --run, each user-space example
# is also executed (20 s timeout) and its exit code reported.
# Run inside the Linux box: bash examples/check.sh [--run]
if [ "$(uname)" != Linux ]; then echo "Pushti vo Linux kutijata: ssh oslab"; exit 1; fi
here="$(cd "$(dirname "$0")" && pwd)"
run=0; [ "$1" == "--run" ] && run=1
tmp="$(mktemp -d)"; trap 'rm -rf "$tmp"' EXIT
pass=0; fail=0
ok()   { echo "PASS: $1"; pass=$((pass+1)); }
bad()  { echo "FAIL: $1"; [ -n "$2" ] && sed 's/^/      /' <<<"$2"; fail=$((fail+1)); }

for dir in "$here"/*/; do
    topic="$(basename "$dir")"
    [ "$topic" == "drivers" ] && continue
    for src in "$dir"*.c; do
        [ -e "$src" ] || continue
        name="$topic/$(basename "$src")"
        out="$(gcc -Wall -Wextra -pthread -o "$tmp/prog" "$src" 2>&1)"
        if [ $? -ne 0 ] || [ -n "$out" ]; then bad "compile $name" "$out"; continue; fi
        ok "compile $name"
        if [ $run -eq 1 ]; then
            mkdir -p "$tmp/run" && (cd "$tmp/run" && timeout 20 "$tmp/prog" </dev/null >"$tmp/out.txt" 2>&1); rc=$?
            want="$(grep -m1 -oE 'expect: signal [0-9]+' "$src" | grep -oE '[0-9]+$')"   # demos that die on purpose
            if [ $rc -eq 124 ]; then bad "run $name (timeout)" "$(tail -5 "$tmp/out.txt")"
            elif [ -n "$want" ] && [ $rc -eq $((128 + want)) ]; then ok "run $name (died with signal $want, as the example intends)"
            elif [ $rc -ge 128 ]; then bad "run $name (signal $((rc-128)))" "$(tail -5 "$tmp/out.txt")"
            else ok "run $name (exit $rc)"; fi
            rm -rf "$tmp/run"
        fi
    done
    for src in "$dir"*.py3 "$dir"*.py; do
        [ -e "$src" ] || continue
        name="$topic/$(basename "$src")"
        out="$(PYTHONPYCACHEPREFIX="$tmp/pycache" python3 -m py_compile "$src" 2>&1)" && ok "syntax $name" || bad "syntax $name" "$out"   # cache goes to tmp, not into the repo
        if [ $run -eq 1 ]; then
            mkdir -p "$tmp/run" && (cd "$tmp/run" && timeout 20 python3 "$src" </dev/null >"$tmp/out.txt" 2>&1); rc=$?
            if [ $rc -eq 124 ]; then bad "run $name (timeout)" "$(tail -5 "$tmp/out.txt")"
            else ok "run $name (exit $rc)"; fi
            rm -rf "$tmp/run"
        fi
    done
done

if [ -d "$here/drivers" ]; then
    kdir="/lib/modules/$(ls /lib/modules 2>/dev/null | head -1)/build"
    if [ -d "$kdir" ]; then
        out="$(make -s -C "$here/drivers" KDIR="$kdir" 2>&1)"
        # "the compiler differs from the one used to build the kernel" is a toolchain note, not a code warning
        if [ $? -eq 0 ] && ! grep -i "warning" <<<"$out" | grep -qv "compiler differs"; then ok "drivers: modules build against $kdir"
        else bad "drivers: module build" "$out"; fi
        make -s -C "$here/drivers" KDIR="$kdir" clean >/dev/null 2>&1
    else
        echo "SKIP: drivers (no kernel headers in /lib/modules)"
    fi
    for src in "$here"/drivers/*_client.c; do
        [ -e "$src" ] || continue
        out="$(gcc -Wall -Wextra -o "$tmp/prog" "$src" 2>&1)"
        if [ $? -ne 0 ] || [ -n "$out" ]; then bad "compile drivers/$(basename "$src")" "$out"; else ok "compile drivers/$(basename "$src")"; fi
    done
fi

echo
echo "Vkupno: $pass PASS, $fail FAIL"
[ "$fail" -eq 0 ] || exit 1
