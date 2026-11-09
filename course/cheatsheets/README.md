# Cheat sheets

Study guides for the OS course, one per topic, written from the labs and exams in this repo. Every function that appears in a sheet appears in a course file, and every code snippet is a real file under `examples/` that compiles and runs in the Linux box (`ssh oslab`). Nothing in them is generic POSIX trivia that the course never asks for; things the course does not use are marked "beyond this course" in one line.

| Sheet | Covers | Exam where it matters |
|---|---|---|
| [shell.md](shell.md) | bash, quoting, globs, `find`, regex, `grep`/`sed`/`awk`, exam recipes | first partial, task 1 |
| [python.md](python.md) | argv and flags, files, strings, dicts, parsing recipes (dates, IPs, logs), regex, `subprocess` | first partial, task 2 |
| [c-toolbox.md](c-toolbox.md) | reading files and words in C, strings, `strtol`, structs for passing data, time, gcc and gdb | every C task |
| [processes.md](processes.md) | `fork`, `exec*`, `wait`/`waitpid`, status macros, `kill`, zombies, timeouts, N-children patterns | first partial task 3, Lab 2 |
| [threads.md](threads.md) | `pthread_create`/`join`, passing arguments, results, mutex and races, one-thread-per-file pattern, fork + threads | first partial task 3, Labs 3 and 5 |
| [pipes.md](pipes.md) | `pipe`, close-the-unused-end rule, `dup2` redirection, building `cmd1 \| cmd2` with exec, parent-child protocols | second partial, Labs 5 and 6 |
| [shared-memory.md](shared-memory.md) | System V `shmget`/`shmat`/`shmctl`, `mmap` file-backed and anonymous, cleanup, sync by sequencing | second partial, Labs 3 and 4 |
| [drivers.md](drivers.md) | character drivers: module skeleton, `file_operations`, `read`/`write` handler contract, `copy_to_user`/`copy_from_user`, buffer bookkeeping patterns, build and test | second partial, one task every year |

## Reading order

**Before the first partial (November):** `shell.md` sections 0–8 and 10, then `python.md`, then `c-toolbox.md`, `processes.md` and `threads.md`. The C task of the first partial has combined processes and threads every year since 2022, so read those two together and do the exam recipes at the end of each.

**Before the second partial (January):** `pipes.md`, `shared-memory.md`, then `drivers.md`. Of the eleven second-partial C files in this repo, five are drivers and six are fork + exec with pipes, shared memory or `mmap`, so the three sheets carry equal weight. In `drivers.md` the buffer patterns (section 6) are the exam.

## Which sheet for which past task

| Task | Sheets |
|---|---|
| `2024/Exams/partial_one/Z1.sh` `.crypto` files merged, calls Z2 | shell |
| `2024/Exams/partial_one/Z2.py3` crypto values, max rise and fall | python §5, §8 |
| `2024/Exams/partial_one/Z3.c` parent reads files to arrays, child creates one thread per array | c-toolbox §2, processes §9, threads §7–8 |
| `2024/Exams/partial_two/Z3.c`, `Z4.c` drivers with a buffer | drivers §4–6, §8 |
| `2023/Exams/partial_one/Z2.py3` IP addresses, flags `a`/`o` | python §5, §8 |
| `2023/Exams/partial_one/Z3.c` one thread per IP, main sums | threads §7, c-toolbox §2 |
| `2022/Exams/partial_one/Z2.py3` word search with option `l`/`w`/`f` | python §1, §8 |
| `2022/Exams/partial_one/Z3.c` word count per file with threads, keyboard fallback | threads §7, processes §9, c-toolbox §2 |
| `2022/Exams/partial_two/Z2.c` fork + exec + pipe | processes §3, pipes §4–5 |
| `2021/Exams/partial_two/Z1.c` fork + exec + shared memory | processes §3, shared-memory §2 |
| `2021/Exams/partial_two/Z2.c` fork + exec + pipe + files | pipes §4–5 |
| `2021/Exams/partial_two/Z3.c` driver | drivers §4–6, §8 |
| `2020/Exams/partial_two/Z3/Z3_2.c` word from stdin vs word from file via `dup2` | pipes §4, §7 |
| `2020/Exams/partial_two/Z2_2.c` shm + `mmap` | shared-memory §2–3 |
| `2020/Exams/partial_two/Z3/Z3.c`, `Z3_3.c` drivers | drivers §4–6, §8 |
| `2020/Exams/partial_two/Z4.c` fork + pipe | pipes §3 |
| `2019/Exams/*/Z2.py3` dates first in line; commands with dash arguments | python §5, §8 |
| `2026/Labs/Lab2` access log; parallel `ping` checker with timeout | python §5, processes §8–9, threads §7 |
| `2025/Labs/Lab2` word swap, `ls -al` dates; N × command | python §8, processes §8 |
| `2025/Labs/Lab3` threads over text; shm | threads §9, shared-memory §6 |
| `2025/Labs/Lab4` shm and `mmap` | shared-memory §2–3, §6 |
| `2025/Labs/Lab5` pipes with `dup2`; threads | pipes §4, threads §9 |
| `2025/Labs/Lab6` `cmd1 \| cmd2` by hand with `fork` + `dup2` + `execvp` | pipes §5, §7 |

## The examples

`examples/<topic>/NN_name.c` or `.py3`, each one self-contained and under 70 lines, with its sheet and section on the first line. Compile and run them yourself, change a line, see what breaks: that is the point of them.

```bash
# in the Linux box, from this folder
bash examples/check.sh          # compiles every C example with -Wall -Wextra -pthread, builds the driver modules, syntax-checks the Python
bash examples/check.sh --run    # additionally runs each user-space example with a 20 s timeout
```

Driver modules compile in the container against the generic kernel headers but cannot be loaded there; `drivers.md` section 7 has the full `insmod` session for a lab PC.
