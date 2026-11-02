#!/bin/bash
# Compile and/or run one solution. Linux or WSL only (fork, pthreads, System V shm).
#   ./run.sh course/2025/Exams/partial_one/Z3.c a.txt b.txt   -> gcc -Wall -pthread into <dir>/bin/Z3, then run
#   ./run.sh course/2025/Exams/partial_one/Z1.sh               -> bash
#   ./run.sh course/2025/Exams/partial_one/Z2.py3 file.txt     -> python3
f="$1"; shift || true
if [ -z "$f" ] || [ ! -f "$f" ]; then
    echo "usage: $0 <file.c|file.sh|file.py3> [args...]"; exit 1
fi
dir=$(dirname "$f"); base=$(basename "$f"); name="${base%.*}"
case "$base" in
    *.c)
        mkdir -p "$dir/bin"
        gcc -Wall -pthread "$f" -o "$dir/bin/$name" || exit 1
        cd "$dir" && exec "./bin/$name" "$@" ;;
    *.sh)       exec bash "$f" "$@" ;;
    *.py|*.py3) exec python3 "$f" "$@" ;;
    *)          echo "unknown file type: $f"; exit 1 ;;
esac
