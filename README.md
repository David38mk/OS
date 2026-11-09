# OS

Solved problems for the Operating Systems course (FEIT): shell scripts, Python scripts and C programs for processes, threads, pipes, shared memory and drivers.

The layout follows the PSA repo: one folder per study year, named by the year it starts in (`course/2026` = 2026/27).

## Layout

```
course/
├── 2026/Labs/                 # 2026/27 - current year, one folder per lab
├── 2025/                      # 2025/26
│   ├── Labs/                  # Lab1-Lab6 (see Labs/README.md)
│   └── Exercises/             # undated practice programs, by topic
├── 2024/Exams/                # partial_one (21.11.2024), partial_two (Jan 2025)
├── 2023/Exams/partial_one/    # 20.11.2023
├── 2022/Exams/                # partial_one (21.11.2022), partial_two (2022/23)
├── 2021/Exams/partial_two/    # 18.01.2022
├── 2020/Exams/partial_two/    # 2020/21
├── 2019/Exams/                # partial_one (19.11.2019), exam_date_1 (second 2019/20 sitting)
└── 2018/Exams/partial_two/    # 30.01.2019
```

Conventions:

- One folder per exam sitting. First partial: `Z1.sh` (Shell), `Z2.py3` (Python), `Z3.c` (C). Second partial: C tasks `Z1.c`, `Z2.c`, ... A task gets its own `Zn/` folder only when it has several files.
- November sitting = `partial_one`, January sitting = `partial_two`, anything else = `exam_date_N` (PSA naming).
- Task text is in the header comment of the file, or in a PDF next to it, when it was saved.
- Build output goes to `bin/` and is ignored by git.

## Running

Everything targets Linux. The quickest way on Windows is the Docker container below; WSL or a lab machine also work:

```bash
./run.sh course/2024/Exams/partial_one/Z3.c btc.txt eth.txt   # gcc -Wall -pthread, then runs bin/Z3
./run.sh course/2024/Exams/partial_one/Z1.sh                  # bash
./run.sh course/2024/Exams/partial_one/Z2.py3 all.crypto      # python3
```

Two-program tasks (`course/2025/Labs/Lab3/threads`) are built by hand: program 1 execs `./lab31`, so compile `lab31.c` to `lab31` in the same folder first.

## Linux test environment (Docker)

`docker-compose.yml` builds an Ubuntu 24.04 container with gcc, gdb, python3 and an SSH server. The repo is bind-mounted at `/home/david/OS`, so you edit on Windows and run inside Linux.

```bash
docker compose up -d --build        # first start builds the image (a few minutes), later starts take seconds
ssh -p 2222 david@localhost         # key auth with ~/.ssh/id_ed25519, fallback password: os
docker exec -it oslab bash          # same shell without ssh
docker compose down                 # stop
```

Inside: `cd ~/OS`, then `./run.sh course/2024/Exams/partial_one/Z3.c ...` or `bash tests/run_tests.sh` in a lab folder. Kernel-module (driver) tasks compile here against the generic kernel headers, but only a real Linux machine can load them.

Full instructions, maintenance and an explanation of how containers, WSL2 and the bind mount work: [docker/README.md](docker/README.md).

## Study material

[course/cheatsheets/](course/cheatsheets/README.md): one cheat sheet per topic (shell, Python, C toolbox, processes, threads, pipes, shared memory, drivers), written from the tasks in this repo, with runnable examples under `course/cheatsheets/examples/` and a map from every past exam task to the sheets it needs.

## Exams

| Folder | Date | Z1 | Z2 | Z3 / notes |
|---|---|---|---|---|
| `2024/Exams/partial_one` | 21.11.2024 | Shell: `.crypto` files containing BTC/Doge/EOS/ETH/XPM, merged into `all.crypto`, calls Z2 | Python: crypto values from a file | C: processes + threads over crypto files |
| `2024/Exams/partial_two` | Jan 2025 | - | - | `Z3.c`, `Z4.c`: character-device drivers (files were named "2kol", exact date not saved) |
| `2023/Exams/partial_one` | 20.11.2023 | Shell: `.ip` files | Python: IP file processing | C: one thread per IP address, counts reachable addresses |
| `2022/Exams/partial_one` | 21.11.2022 | Shell: 3+ args, runs a `.py3` script (default `prva.py3`) | Python: 4 args (file was dated 20.11, probably a typo) | C: word count across files with threads; `Z3_v2.c` is an earlier draft and does not compile |
| `2022/Exams/partial_two` | 2022/23 | - | `Z2.c`: fork + exec + pipe | - |
| `2021/Exams/partial_two` | 18.01.2022 | C: fork + exec + shared memory | C: fork + exec + pipe + files | C: driver |
| `2020/Exams/partial_two` | 2020/21 | - | `Z2_2.c`: shm + mmap | `Z3/`: file, mmap and pipe variants (`Z3.c`, `Z3_1.c`..`Z3_3.c`); `Z4.c`: fork + pipe |
| `2019/Exams/partial_one` | 19.11.2019 | Shell: `naredbi.txt` | Python | C task not saved |
| `2019/Exams/exam_date_1` | 2019/20, date unknown | Shell: finds files, calls `./python_script.py3` (that is `Z2.py3`), counts dates | Python: dates per month | - |
| `2018/Exams/partial_two` | 30.01.2019 | Shell: `.ecf` directories | - | - |
