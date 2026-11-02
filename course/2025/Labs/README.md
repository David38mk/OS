# OS Labs 2025/26

All lab work from before 2026/27. Two lab series ended up here (the task sheets changed between years), so where two different "Lab N" tasks exist they sit in topic sub-folders.

| Lab | Files | Topic |
|---|---|---|
| Lab1 | `2024_2025_OS-Lab1-Interpreter_Shell_ShellScript.pdf` | Shell interpreter and scripts. Text only, no solution saved. |
| Lab2 | `Z1.py3`, `Z2.py3`, `Z3.c`, PDF | Python scripts over files and arguments; `Z3.c`: fork + exec. |
| Lab3 | `threads/Lab3.c`, `threads/lab31.c`, PDF | Threads: program 1 searches a file for a number with N threads, then execs program 2 (`./lab31`). Compile `lab31.c` to `lab31` first. |
| Lab3 | `shm/Z1.c`, `shm/Z2.c` | Three process types communicating through shared memory: System V shm (Z1) and mmap (Z2). Task text in the header comment. |
| Lab4 | `Z1.c`, `Z2.c` | Shared memory: System V shm (Z1) and mmap (Z2). |
| Lab5 | `pipes/Z1.c` | Three commands over pipe files with `dup2`, run as `./Z1 1`, `./Z1 2`, `./Z1 3`. |
| Lab5 | `threads/Z1.c` | Threads reading a file given as argument through mmap. |
| Lab6 | `Z1.c`, `Z2.c` | Pipes: Z1 pipe + files, Z2 fork + exec + pipe. |

Build from the repo root with `./run.sh course/2025/Labs/Lab4/Z1.c`, or by hand with `gcc -Wall -pthread Z1.c -o bin/Z1` (Linux or WSL).
