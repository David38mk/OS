# OS Labs 2026/27

| Lab | Topic | Text | Solutions |
|---|---|---|---|
| Lab1 | Shell interpreter and scripts | `Lab1/Lab1.md` (from the .docx next to it) | Homework: `Doma_Z1.sh` (start and kill firefox), `Doma_Z2.sh` (grep for `.c` files), `Doma_Z3.sh` (sed censors valid e-mails in `vezba12.txt`). Class: `Z1.sh` (percent of given files found), `Z2.sh` (`naredbi.txt` lookup and cleanup), `Z3.sh` (copy `.py`/`.py3` files containing `password`, print the lines). |
| Lab2 | Python and processes | `Lab2/Lab2.md` (from the approved .pdf next to it; the first version of the text is in `Lab2/old_text/`) | `Z1.py3` (access-log statistics, suspicious IPs, `-b` blocklist, with `split()`; `Z1_S.py3` is the regex version), `Z2.c` (parallel `ping` checker with timeout, one `main`, `time()`; `Z2_S.c` adds `-r M` retries, a per-round function and a monotonic clock), `Z2_threads.c` (bonus 2: waiting done by threads). `Lab2/portal/` holds the files published for students. |

Run from the lab folder, for example `bash Z1.sh a.txt b.txt` or `python3 Z1.py3 portal/access.log`. C programs: `gcc -Wall -pthread Z2.c -o Z2`. `bash tests/run_tests.sh` builds a temporary directory tree, runs every script against it and prints PASS/FAIL per check. `Doma_Z1.sh` needs a desktop with firefox, so it is only syntax-checked.
