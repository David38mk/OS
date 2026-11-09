# Processes Cheat Sheet – study guide for the OS labs and exams

How a C program creates, runs, waits for and stops other programs on Linux (`fork`, `exec`, `wait`, `kill`). Read sections 0 to 7 in order once, use 8 as a pattern library, 9 before an exam, 10 when something misbehaves.

Every code block marked with a file name is a real program in `examples/processes/`. Compile and run them in the Linux box (the `oslab` container) with:

```bash
$ gcc -Wall -Wextra -pthread examples/processes/01_fork_twice.c -o /tmp/a && /tmp/a
```

All of them compile with zero warnings. Output pasted below was produced by that command. Task quotes are in Macedonian, exactly as in the course files.

---

## 0. What a process is and why we create them

A **process** is a running program: its code, its own memory, its open files, and a number (the **PID**). The system starts one program, you want it to start others (run a command N times, check many hosts at once). You do that with three calls that always work as a team:

| Call | Job |
|---|---|
| `fork()` | make a copy of me; now there are two processes |
| `exec...()` | replace the program inside this process with another program |
| `wait...()` | block until a child ends and collect how it ended |

```
 parent (pid 100)
    |
    |-- fork() --------> child (pid 101)         copy of the parent
    |                        |
    |                        |-- execvp("ping", ...)   same pid 101, now runs ping
    |                        |
    |-- waitpid(101) <-------+-- ping ends: exit code 0
    |
    v   parent continues
```

---

## 1. The process model

| Term | Meaning | Get it |
|---|---|---|
| PID | process id, unique while the process lives | `getpid()` |
| PPID | pid of the parent that created it | `getppid()` |
| address space | the process's private memory: code, globals, heap, stack | |
| child | a process created by `fork` of this process | the return value of `fork` |

After `fork` the child has **a copy** of the parent's memory: same variables, same values, same open files, same position in the code. From then on the two copies are independent. (The kernel does not really copy all pages at once: it shares them and copies a page only when someone writes to it, called copy-on-write. You never see the difference.)

`fork` is the only function that **returns twice**: once in each process, with a different value.

```
              before fork                      after fork
          +----------------+            +----------------+   +----------------+
          |  one process   |            | PARENT         |   | CHILD          |
          |  pid 1858      |  fork()    | pid 1858       |   | pid 1859       |
          |  p = ?         | ---------> | p = 1859       |   | p = 0          |
          |  counter = 10  |            | counter = 10   |   | counter = 10   |
          +----------------+            +----------------+   +----------------+
                                           separate memory, no sharing
```

| Function | Signature | Returns |
|---|---|---|
| `getpid` | `pid_t getpid(void)` | pid of the caller, never fails |
| `getppid` | `pid_t getppid(void)` | pid of the parent (becomes `1` or a subreaper's pid if the parent died) |

Headers: `<unistd.h>` (fork, exec, getpid), `<sys/types.h>` (`pid_t`), `<sys/wait.h>` (wait).

`examples/processes/01_fork_twice.c`, run:

```
before fork: pid=3137 ppid=3131
CHILD : fork returned 0, pid=3138 ppid=3137
PARENT: fork returned 3138 (the child's pid), pid=3137
```

The child's `ppid` is the parent's `pid`. The parent got the child's pid as the return value: that is the only way it learns it.

Memory is copied, not shared (`02_fork_memory.c`, the child adds 5, the parent waits and looks):

```
child : counter=15 local=105 (&counter=0x55edd82fb010)
parent: counter=10 local=100 (&counter=0x55edd82fb010)
```

Same address, different values: each process has its own copy of that address. Consequence for exams: a result computed in a child is **not** visible in the parent. Passing data back needs a pipe, shared memory, a file, or the exit code (0 to 255). See section 9, recipe 3.

---

## 2. fork

| | |
|---|---|
| Signature | `pid_t fork(void)` |
| Args | none |
| Returns | `< 0` failure, no child created (`errno` set); `0` you are the child; `> 0` you are the parent, value is the child's pid |
| Errors | `EAGAIN` too many processes, `ENOMEM` no memory |

Always three-way branch:

```c
pid_t p = fork();
if (p < 0) { perror("fork"); return 1; }          // failed
if (p == 0) { /* child code */ _exit(0); }          // child: do the job, then LEAVE
/* parent code continues here, p is the child's pid */
```

**Order is not defined.** After `fork` both processes are runnable; the scheduler picks. `03_fork_order.c` forks 3 children, each prints one line, the parent prints one line. Run 300 times, counted:

```
264  child 0, child 1, parent, child 2
 15  child 0, parent, child 1, child 2
 11  child 0, child 1, child 2, parent
  3  child 1, parent, child 2, child 0
  3  child 1, child 0, parent, child 2
  3  child 0, parent, child 2, child 1
  1  parent, child 1, child 0, child 2
```

Seven different orders from the same program. Never write code (or an exam answer) that relies on one of them. If order matters, use `wait`.

**Fork in a loop, the correct way.** The child must leave the loop (`_exit` or `exec`), otherwise it keeps looping and forks too. `04_fork_loop.c`:

```c
for (int i = 0; i < N; i++) {
    pid_t p = fork();
    if (p < 0) { perror("fork"); break; }
    if (p == 0) {
        printf("child %d from iteration %d\n", getpid(), i);
        fflush(stdout);
        if (!bad) _exit(0);        // correct: the child leaves the loop at once
    }
}
while (wait(NULL) > 0) ;
```

```
$ ./04_fork_loop | wc -l        # 3 children
3
$ ./04_fork_loop bad | wc -l    # children keep forking
7
```

Without the exit, every process runs the remaining iterations: after N iterations there are 2^N processes (2^N - 1 children). With N = 3 that is 7 children; with N read from the keyboard and mistyped as 30 it is a fork bomb that freezes the machine.

The other correct shape is `break`: `if (p == 0) break;` and after the loop `if (i < N) { child work }`. The course files use it (`NizaNaredbi.c`, `1000br.c`). Both work, just make sure the child can never reach the parent's code.

---

## 3. exec: run another program

`exec` throws away the current program in this process and loads another one into it. The pid stays. Open files stay. Memory, variables and code are replaced. It is normally called **in the child** after `fork`.

The names say how you pass the arguments:

| Function | Signature | Arguments as | Searches `PATH`? |
|---|---|---|---|
| `execl` | `int execl(const char *path, const char *arg0, ..., (char *)NULL)` | list | no, `path` must be a path |
| `execlp` | `int execlp(const char *file, const char *arg0, ..., (char *)NULL)` | list | yes |
| `execv` | `int execv(const char *path, char *const argv[])` | array | no |
| `execvp` | `int execvp(const char *file, char *const argv[])` | array | yes |
| `execvpe` | `int execvpe(const char *file, char *const argv[], char *const envp[])` | array | yes, plus your own environment (GNU extension) |

Memory aid: **l** = list (you type the args in the call, count known at compile time), **v** = vector (an array, count known at run time), **p** = looks in `PATH` like the shell does (`ping` instead of `/usr/bin/ping`).

| Rule | Why |
|---|---|
| `arg0` / `argv[0]` is the program name, repeated | by convention the program sees its own name as `argv[0]`; `execlp("ls", "ls", "-l", NULL)` |
| the list **must end with `(char *)NULL`** | exec does not know where the list ends otherwise (cast needed in the variadic calls) |
| on success exec **never returns** | the code after it is gone |
| it returns `-1` only on failure | so code right after exec is the error path: `perror` + `_exit(127)` |

`05_exec_family.c` runs the same `echo` four ways:

```c
char *v[] = {"echo", "via", which == 2 ? "execv" : "execvp", NULL};
case 0: execl("/bin/echo", "echo", "via", "execl", (char *)NULL); break;
case 1: execlp("echo", "echo", "via", "execlp", (char *)NULL); break;
case 2: execv("/bin/echo", v); break;
case 3: execvp("echo", v); break;
perror("exec");  _exit(127);        // only reached if exec failed
```

```
via execl
via execlp
via execv
via execvp
```

Exit code **127** is the shell's convention for "command not found". Use it after a failed exec so the parent can tell "the command ran and failed" from "the command never started".

### The command given on the command line, unknown number of arguments

The course asks for this twice. `./a.out ls -l /tmp`: the command is `argv[1]`, its arguments are `argv[2]...`. `execvp(argv[1], &argv[1])` passes the tail of your own `argv`; C guarantees `argv[argc] == NULL`, so the terminator is already there. `06_exec_cmdline.c`:

```c
if (p == 0) {
    execvp(argv[1], &argv[1]);
    perror(argv[1]);                // exec returned, so it failed
    _exit(127);
}
```

```
$ ./06_exec_cmdline echo a b c
a b c
command exited with 0
$ ./06_exec_cmdline nosuchcmd
nosuchcmd: No such file or directory
command exited with 127
$ ./06_exec_cmdline sh -c 'exit 3'
command exited with 3
```

### Running another compiled program

`course/2025/Labs/Lab3/threads/Lab3.c`, program 1 starts program 2 (`lab31`, compiled from `lab31.c` in the same directory) and passes N and K:

```c
PIDvtora = fork();
if(PIDvtora==0){
  execl("./lab31","lab31",argv[1],argv[2],NULL);
}
```

`./lab31` has a slash, so it is a path relative to the current directory: with `execl` (no `p`) that is exactly what you want. The program name must exist and be executable (`gcc lab31.c -pthread -o lab31` first). Inside `lab31`, `getppid()` is program 1's pid, so program 2 knows whom to `kill`. Two kind remarks: the NULL should be `(char *)NULL`, and nothing follows the `execl`, so if the exec fails the child continues as a second copy of program 1. Add `perror` and `_exit(127)`.

`07_exec_self.c` shows the same shape with one file: the program starts itself with a different argument:

```
program 2: started with arg '42', my parent is 3178
program 1: my pid is 3178, child is 3179
```

---

## 4. wait and waitpid

A parent must collect each child's end. Until it does, the finished child stays in the process table as a **zombie** (section below).

| Function | Signature | Meaning | Returns | Errors |
|---|---|---|---|---|
| `wait` | `pid_t wait(int *status)` | block until **any** child ends | its pid, `-1` if there are no children | `ECHILD` no children |
| `waitpid` | `pid_t waitpid(pid_t pid, int *status, int options)` | wait for a **specific** child (`pid > 0`) or any (`-1`) | pid of the reaped child; `0` with `WNOHANG` when nobody ended yet; `-1` on error | `ECHILD` not your child / already reaped, `EINTR` interrupted |

`status` may be `NULL` if you do not care how it ended. `options`: `0` blocks, `WNOHANG` returns at once.

Decode `status` with macros, never by hand:

| Macro | True when | Then |
|---|---|---|
| `WIFEXITED(st)` | the child ended with `exit`/`return` | `WEXITSTATUS(st)` is the exit code |
| `WIFSIGNALED(st)` | a signal killed it | `WTERMSIG(st)` is the signal number |

An **exit code** is one byte: 0 to 255. 0 means success. `_exit(300)` gives `300 & 255 = 44`. `08_wait_status.c` makes three children (`_exit(0)`, `_exit(300)`, `raise(SIGKILL)`):

```
child 0: exited, code 0
child 1: exited, code 44
child 2: killed by signal 9
```

**Wait for all N children.** Two correct ways:

```c
for (int i = 0; i < N; i++) wait(NULL);          // you know there are exactly N
while (wait(NULL) > 0) ;                          // robust: loops until ECHILD (-1)
```

`14_parallel.c` starts 4 children that finish in reverse order, then the parent reaps them:

```
child 3 done
child 2 done
child 1 done
child 0 done
parent: reaped 4 children
```

Use `waitpid(kid[i], ...)` when you must know **which** child gave which result (health check, per-command timing, section 6).

### Zombie and orphan

A **zombie** is a child that has ended but whose parent has not called `wait` yet. It uses no CPU or memory, only a slot in the process table, but if the parent never waits it stays until the parent ends. `ps` shows it as state `Z`, `<defunct>`. `09_zombie.c` (child exits at once, parent sleeps 1 s, then runs `ps`, then `wait`):

```
before wait():
  PID  PPID STAT COMMAND
 3185  3184 Z    09_zombie
after wait():
  PID  PPID STAT COMMAND
pid 3185: gone
```

An **orphan** is the opposite: the parent ended first. The system gives the child to another process (classically `init`, pid 1), so `getppid()` changes. `10_orphan.c`:

```
parent: my pid is 1911, exiting without wait()
child : my parent is 1911
child : my parent is now 1 (not the original one)
```

(In some desktop sessions the new parent is a "subreaper" with another pid instead of 1.) An orphan is not a problem: its new parent reaps it. A zombie that never goes away is a bug in the parent.

---

## 5. kill

| Function | Signature | Returns | Errors |
|---|---|---|---|
| `kill` | `int kill(pid_t pid, int sig)` | `0` on success, `-1` on error | `ESRCH` no such process, `EPERM` not allowed |

| Signal | Number | Meaning |
|---|---|---|
| `SIGTERM` | 15 | "please stop"; the program may catch it and clean up (default action: die) |
| `SIGKILL` | 9 | stop **now**; cannot be caught or ignored |
| `SIGINT` | 2 | what Ctrl+C sends |
| `0` | 0 | not a signal: only checks that the process exists and you may signal it |

`11_kill.c`:

```
alive? kill(p, 0) = 0
child ended by signal 15 (SIGTERM is 15)
kill(p, 0) now fails with ESRCH: no such process
second child: signal 9 (SIGKILL is 9)
```

Always `waitpid` after killing: the killed child is a zombie until you do (`WTERMSIG(st)` tells the parent which signal ended it). In the exam "kill the other process" (Lab 3) use `SIGKILL`; for a polite stop `SIGTERM`. Careful: `kill(0, ...)` or `kill(-1, ...)` signal whole groups or everything; never pass an unchecked pid.

Beyond this course: catching signals with `signal()` or `sigaction()` handlers.

---

## 6. Timing

Task: "колку секунди траела проверката". Two clocks:

| Call | Resolution | Use |
|---|---|---|
| `time(NULL)` | whole seconds, follows the wall clock | printing a date, **not** measuring |
| `clock_gettime(CLOCK_MONOTONIC, &ts)` | nanoseconds, never jumps | measuring durations and timeouts |

`time()` ticks on whole seconds, so a 0.3 s job measures as 0 s or 1 s depending on where the tick falls. `course/2026/Labs/Lab2/Z2_S.c` explains why that breaks a timeout:

```c
// Sekundi od monoton chasovnik, so delovi od sekunda. time() ne e dobro za
// merenje na T: ima rezolucija od 1 s, pa "pominaa 2 s" bi vazhelo vekje po
// 1.01 s realno vreme i deteto bi bilo ubieno predvreme. Monotoniot chasovnik
// ne skoka ni koga sistemot go menuva datumot/vremeto.
static double now_s(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + ts.tv_nsec / 1e9;
}
```

`12_timing.c` (child works 0.3 s):

```
time():      0 s          (can also print 1: it depends on when the second changes)
monotonic:   0.30 s
```

`struct timespec` has `tv_sec` and `tv_nsec`. `<time.h>`.

**Time each child separately:** record `start[i]` right after its `fork`, and when `waitpid(kid[i], ...)` returns, `now - start[i]` is that child's time. Do not time the whole loop once. `course/2025/Exercises/processes/NizaNaredbi.c` does this with a thread per child (section 9, recipe 6, and `threads.md` section 7).

Sleeping: `sleep(s)` whole seconds, `usleep(us)` microseconds, `nanosleep(&ts, NULL)` precise. Beyond this course: `alarm`, timers.

---

## 7. stdio and fork: the duplicated prompt

`printf` does not write immediately. It puts text in a buffer inside your process and writes when the buffer is full, when it sees `\n` **on a terminal**, or on `fflush`/`exit`. When the output goes to a pipe or a file, the buffer is flushed only when full or at exit. `fork` copies the buffer too. Both processes later flush their copy, so the text appears twice. `13_stdio_dup.c` prints a prompt without `\n`, forks, the child prints and calls `exit`:

```
$ ./13_stdio_dup | cat          # output is a pipe: buffered
Vnesi N: [child] Vnesi N: [parent]
$ ./13_stdio_dup flush | cat    # fflush(stdout) before fork
Vnesi N: [child] [parent]
```

On a terminal you would not see it (a terminal is line buffered), which is why the bug shows up only in the tester that captures your output. Rules:

| Rule | Why |
|---|---|
| `fflush(stdout)` **before every `fork`** (especially after a prompt without `\n`) | the child must not inherit unwritten text |
| in a child that only runs `exec` or fails: `_exit(127)` | `exit()` would flush the inherited copy of the parent's buffer |
| in a child that printed its own text: `fflush(stdout);` then `_exit(0)` | `_exit` skips flushing: unflushed text is lost (try it in a pipe) |
| `exit(n)` / `return` from `main` flush stdio, run `atexit` handlers | fine when the buffer was empty at fork time |
| `freopen("/dev/null", "w", stdout);` in the child before exec | silences the program it will run (`ping`); do stderr the same way |

`course/2026/Labs/Lab2/Z2_S.c` does all three:

```c
fflush(stdout);                                  // before fork
pid[i] = fork();
if (pid[i] == 0) {
    freopen("/dev/null", "w", stdout);
    freopen("/dev/null", "w", stderr);
    execlp("ping", "ping", "-c", "1", hosts[idx[i]], (char *)NULL);
    _exit(127);                                  // exit() would flush the parent's copy
}
```

Note: `exit` and `_exit` differ for the child, but `exec` replaces the whole buffer, so after a **successful** exec the old buffer is gone and nothing duplicates.

---

## 8. Patterns

Each is a complete program in `examples/processes/`. Output below is real.

### 8.1 N children in parallel, wait for all (`14_parallel.c`)

Fork all first, wait after. The children run at the same time; see section 4 for the output. Total time = the slowest child.

### 8.2 Sequential: fork, wait, next (`15_sequential.c`)

`waitpid(p, NULL, 0)` inside the loop, so the next child starts only when this one ended. Total time = the sum.

```
child 0 starts
child 0 ends
child 1 starts
child 1 ends
child 2 starts
child 2 ends
```

### 8.3 Pool: at most K running (`16_pool.c`, N = 6, K = 2)

```c
int running = 0;
for (int i = 0; i < N; i++) {
    if (running == K) { wait(NULL); running--; }      // full: wait for any one to finish
    pid_t p = fork();
    if (p == 0) { /* job i */ fflush(stdout); _exit(0); }
    running++;
}
while (wait(NULL) > 0) ;                               // the last ones
```

```
job 0 start
job 1 start
job 0 end
job 1 end
job 2 start
job 3 start
...
```

### 8.4 Chain: each child forks the next (`17_chain.c`)

The parent of each link waits and stops; the child becomes the next link:

```c
while (depth < DEPTH) {
    pid_t p = fork();
    if (p > 0) { waitpid(p, NULL, 0); return 0; }     // parent link: wait, then finish
    depth++;                                           // child: next link, loops again
    printf("depth %d: pid=%d ppid=%d\n", depth, getpid(), getppid());
    fflush(stdout);
}
```

```
depth 0: pid=3474 ppid=3131
depth 1: pid=3475 ppid=3474
depth 2: pid=3476 ppid=3475
depth 3: pid=3477 ppid=3476
```

Each ppid is the previous pid: a chain, not a fan. The loop here is the opposite of 8.1: the **parent** leaves, the child continues.

### 8.5 Parent timeout: poll with `WNOHANG`, then kill (`18_timeout.c`)

```c
for (;;) {
    pid_t r = waitpid(p, &st, WNOHANG);     // 0 = still running, p = ended, -1 = error
    if (r == p)  { printf("finished\n"); break; }
    if (r == -1) { perror("waitpid"); break; }
    clock_gettime(CLOCK_MONOTONIC, &t);
    if (t.tv_sec - t0.tv_sec >= T) {
        kill(p, SIGKILL);
        waitpid(p, &st, 0);                 // reap, otherwise a zombie
        printf("TIMEOUT after %d s, child killed (signal %d)\n", T, WTERMSIG(st));
        break;
    }
    usleep(100000);                         // 100 ms between checks, not a busy loop
}
```

```
TIMEOUT after 2 s, child killed (signal 9)
```

The same idea for many children at once is `course/2026/Labs/Lab2/Z2_S.c` (`run_round`): loop over all running children, `waitpid(pid[i], &st, WNOHANG)` each, kill the ones past the limit, `nanosleep` 100 ms, repeat until none are left. 100 ms and not 1 s, so a fast host shows `0 s` instead of `1 s`.

### 8.6 Parent reads N from the keyboard, then forks N (`19_read_n.c`)

The shape of Lab 2 2025 Z3:

```c
printf("Vnesi N: ");
fflush(stdout);                                    // prompt out BEFORE any fork
if (scanf("%d", &N) != 1 || N < 1 || N > 100) { fprintf(stderr, "N must be 1..100\n"); return 1; }
pid_t kid[N];                                      // a VLA is fine once N is validated
for (int i = 0; i < N; i++) {
    kid[i] = fork();
    if (kid[i] == 0) { printf("child %d (pid %d)\n", i, getpid()); fflush(stdout); _exit(0); }
}
for (int i = 0; i < N; i++) waitpid(kid[i], NULL, 0);
```

```
$ echo 3 | ./19_read_n
Vnesi N: child 0 (pid 3482)
child 1 (pid 3483)
child 2 (pid 3484)
Zavrshuva process 0
$ echo abc | ./19_read_n
N must be 1..100
```

### 8.7 Fork + exec of another program in the same directory

`07_exec_self.c` (section 3) and `course/2025/Labs/Lab3/threads/Lab3.c`.

---

## 9. Exam recipes

### Recipe 1. "Run the command N times" (Lab 2 2025 Z3)

> „Да се напише C програма која ќе направи N пати извршување на некоја наредба. Родител процесот најпрво од тастатура го вчитува бројот колку пати ќе се изврши наредбата. Самата наредба се внесува како аргумент од командна линија и може да има непознат (променлив) број на аргументи.“

1. `argc < 2` → usage message to `stderr`, `return 1`.
2. Print the prompt, `fflush(stdout)`, `scanf("%d", &N)`, check it returned 1 and `N >= 1`.
3. Loop `i < N`: `fork`; if `< 0` `perror` and `break`.
4. In the child: `execvp(argv[1], &argv[1]); perror(argv[1]); _exit(127);`.
5. After the loop: `while (wait(NULL) > 0) ;`.
6. `printf("Zavrshuva process 0\n")`.

Full verified solution: `examples/processes/20_nxcmd.c`:

```
$ echo 3 | ./20_nxcmd echo hi there
Vnesi kolku pati: hi there
hi there
hi there
Zavrshuva process 0
```

Your own `course/2025/Labs/Lab2/Z3.c` has the right skeleton. Three small things to fix: `exit(0)` needs `#include <stdlib.h>` (gcc warns), the child that fails exec reports success (`exit(0)` hides it: use `_exit(127)`), and the prompt has no `\n`/`fflush` before the fork.

### Recipe 2. Health check with timeout and `-r` (Lab 2 2026 Z2)

> „Родител процесот најпрво од тастатура вчитува колку секунди (T) најмногу смее да трае една проверка. Потоа за секој хост креира по едно дете, а секое дете ја извршува наредбата `ping -c 1 <хост>`. Родителот не чека бесконечно: доколку некое дете не заврши за T секунди, родителот го прекинува (убива).“

1. Parse `-r M` first (`strcmp(argv[1], "-r")`), the hosts are the rest. No hosts → error.
2. Read T, check `>= 1`.
3. For each host: `fflush(stdout)`, `fork`, child: silence stdout and stderr with `freopen`, `execlp("ping", "ping", "-c", "1", host, (char *)NULL)`, `_exit(127)`. Parent stores `pid[i]` and a monotonic start time.
4. Loop until none left: `waitpid(pid[i], &st, WNOHANG)` for each still-running child; ended → `UP` if `WIFEXITED && WEXITSTATUS == 0`, else `DOWN`; past T → `kill(SIGKILL)`, `waitpid`, `TIMEOUT`. `nanosleep` 100 ms.
5. `-r M`: repeat steps 3-4 for the hosts that are DOWN, up to M tries, `sleep(1)` between rounds.
6. Print one line per host **in argument order** (results array indexed by host), then `Zavrshuva process 0`.

Full solution with all of that: `course/2026/Labs/Lab2/Z2_S.c`. The thread variant (`Z2_threads.c`) is in `threads.md` section 7.

### Recipe 3. Parent reads files, child makes threads (2024 partial one, Z3)

> „Главниот процес треба да креира онолку низи со реални вредности колку што има датотеки … Потоа, главниот процес креира дете процес на кој му ги испраќа овие низи од крипто-валути и го чека дете процесот да заврши. Дете процесот најпрво треба да креира онолку нитки колку што има низи …“

The text says the parent "sends" the arrays to the child. You do **not** need a pipe: `fork` copies the whole memory, so if the parent fills the global arrays **before** `fork`, the child already has them. The reverse does not work: the child's results are in the child's copy; the parent sees nothing (it only gets the exit code). That is fine here because the child itself prints the results. `threads.md` section 8 has a verified program (`examples/threads/13_fork_then_threads.c`) that shows exactly this:

```
child : array 0 largest = 4
child : array 1 largest = 6
child : array 2 largest = 6
parent: best[0] = 0 (the child's result never came back)
```

1. Parent: read every file into a global array, remember sizes.
2. `fork`. `< 0` → error.
3. Child: create one thread per array (index via an array, not `&i`), join them, print, `_exit(0)`.
4. Parent: `waitpid(child, NULL, 0)`.

Bonus in that task: print with the shell command `echo` instead of `printf`: in the child `execlp("echo", "echo", text, (char *)NULL)` (`exec` replaces the whole process, so it must be the last thing that process does).

### Recipe 4. Keyboard fallback when arguments are missing (2022 partial one, Z3)

> „Доколку не добие листа од датотеки од командна линија (добие само збор за пребарување), програмата треба да му дозволи да внесе имиња на 3 датотеки од тастатура. Доколку нема ни збор за пребарување, најпрво го внесува зборот од тастатура, а потоа имињата на 3те датотеки.“

Decide by `argc` and fill the **same** variables, so everything after is one code path:

| `argc` | word from | files from |
|---|---|---|
| 1 | keyboard | 3 names from keyboard |
| 2 | `argv[1]` | 3 names from keyboard |
| >= 3 | `argv[1]` | `argv[2]...argv[argc-1]` |

`scanf("%255s", buf)` with a width limit, check `== 1` after each. `course/2022/Exams/partial_one/Z3.c` does exactly this (it uses threads, `threads.md` section 7).

### Recipe 5. K commands with random waits (`course/2025/Exercises/processes/Knaredbi.c`)

Read K, read K commands (command + up to 3 arguments), fork K children; each child sleeps a random 1 to 20 s, prints how long it waited, then `execlp`s its command; the parent waits for all and prints the total wait.

Kind remarks on the file, all real lessons:

1. `#include <phtread.h>` is a typo (it also is not needed): the file does not compile. `time()` needs `<time.h>`.
2. After the loop `i == K`, so `if(i>K)` is never true: the parent never waits and never prints the total. Delete the `if`.
3. `brojac+=count` runs in the **child**: it changes the child's copy of `brojac`, the parent's stays 0. To sum the waits the parent needs another channel (exit code of the child holds the 1..20 seconds, so `WEXITSTATUS` works; or a pipe).
4. The `printf` has no `\n`/`fflush` and is followed by `exec`: when output is a pipe or file, the buffered text is thrown away by the exec and never appears.
5. Nothing follows the `execlp`: if it fails the child falls into the parent's loop and forks more children.

### Recipe 6. Threads that wait for processes (`course/2025/Exercises/processes/NizaNaredbi.c`)

Each argument is a command; fork one child per command (child `break`s out of the loop and `execlp`s), the parent creates one thread per child; each thread polls `waitpid(pid, NULL, WNOHANG)` once per second and stores the number of seconds. Discussion and the blocking version: `threads.md` section 7. Two pitfalls: the time is measured in whole seconds because of `sleep(1)` polling (the first check comes before the command ends, so a 0.2 s command shows `1`, a 1.1 s one shows `2`), and if `execlp` fails the child returns from `main` as a copy of the parent.

---

## 10. Pitfalls

| # | Symptom | Cause | Fix |
|---|---|---|---|
| 1 | `ps` shows `<defunct>` children, or you see zombies after your program ran for long | the parent never called `wait`/`waitpid` | wait for every child; after `kill`, `waitpid` too (section 4) |
| 2 | Output repeated, processes multiply, program "runs again" after an error | the child falls through into the parent's code, e.g. nothing after a failed `execlp` | `perror` + `_exit(127)` right after every exec; `_exit(0)` at the end of every child branch |
| 3 | `exec` crashes or the program receives garbage extra arguments (undefined behaviour) | the argument list is missing `(char *)NULL` at the end | always end the list with `(char *)NULL`; with `execvp(argv[1], &argv[1])` it is already there |
| 4 | Parent prints 0 / old value after the child "computed" something | memory is copied, not shared (section 1) | pass results with the exit code (0-255), a file, a pipe, or shared memory |
| 5 | Child gets no input or reads the wrong line after `scanf` | `scanf` reads a whole block of stdin into a buffer; a child that reads stdin afterwards starts after what the parent already buffered and misses lines | read all input in the parent first and pass values in variables |
| 6 | Prompt appears twice in the tester | printed without `\n`, then forked; output was a pipe (section 7) | `fflush(stdout)` before `fork` |
| 7 | Text printed by a child vanishes when output is piped | child ended with `_exit` without flushing | `fflush(stdout)` before `_exit`, or `exit` when the buffer was empty at fork |
| 8 | Order of lines differs between runs; works "on my machine" | scheduling order of parent and child is undefined (section 2) | order with `wait`; never with `sleep` |
| 9 | Program is slow, or still racy after adding `sleep(1)` "to order" things | `sleep` guesses, does not synchronise | use `waitpid`; the only legitimate sleeps are polling intervals and the "pause between tries" tasks ask for |
| 10 | the child continues with code meant for the parent after a `return` | `return` in `main` ends the process, but `return` inside a helper function only goes back to the caller, which then runs on as a "second parent" | in a child always `_exit(n)` (or `exit(n)`), which ends the process from any depth |
| 11 | Machine freezes after typing a number | fork in a loop without child exit: 2^N processes (section 2); or N read from the keyboard and not checked | child `_exit`/`exec`; validate `N` (`1..100`) before any loop |
| 12 | Crash or garbage with `pid_t kid[N]` | `N` from `scanf` unchecked: 0, negative or huge makes a VLA of bad size, or `N` stays uninitialised when the input is not a number | `if (scanf("%d",&N)!=1 \|\| N<1 \|\| N>100) return 1;` first |
| 13 | A killed child is still listed | killed but not waited | `kill` then `waitpid` |
| 14 | `kill(pid, SIGKILL)` hits a stranger | the pid was reaped and reused by another process | only kill pids you have not reaped yet; check `waitpid` result first |
| 15 | Exit code of the parent is always 0 | `exit(0)` after a failed exec hides the error | `_exit(127)` and test `WEXITSTATUS` in the parent |

Real bugs of this kind in the repo files, with kind comments: section 9, recipes 1, 5 and 6; `course/2025/Labs/Lab3/threads/Lab3.c` (fork + exec without error path).

Beyond this course: signal handlers, pipes between processes (see `pipes.md`), shared memory (see `shared-memory.md`).

---

## 11. Quick reference

| Task | Call | Note |
|---|---|---|
| create a child | `pid_t p = fork();` | `<0` error, `0` child, `>0` parent |
| my pid, parent pid | `getpid()`, `getppid()` | |
| run a program (path) | `execl("/bin/ls", "ls", "-l", (char *)NULL)` | list of args |
| run a program (PATH) | `execlp("ls", "ls", "-l", (char *)NULL)` | list, searches `PATH` |
| run with array | `execv(path, argv)`, `execvp(name, argv)` | array ends with `NULL` |
| command from argv | `execvp(argv[1], &argv[1])` | then `perror`, `_exit(127)` |
| wait for any child | `wait(NULL)`; `while (wait(NULL) > 0);` | returns `-1` when none left |
| wait for one child | `waitpid(pid, &st, 0)` | blocks |
| check without blocking | `waitpid(pid, &st, WNOHANG)` | `0` = still running |
| how did it end | `WIFEXITED`/`WEXITSTATUS`, `WIFSIGNALED`/`WTERMSIG` | exit code is 0-255 |
| stop a child | `kill(pid, SIGKILL)` then `waitpid` | `SIGTERM` is the polite one |
| does the process exist | `kill(pid, 0) == 0` | `ESRCH` if gone |
| measure time | `clock_gettime(CLOCK_MONOTONIC, &ts)` | not `time()` |
| pause | `sleep(s)`, `usleep(us)`, `nanosleep` | never to "order" processes |
| leave the child | `_exit(n)` | flush stdio first if the child printed |
| silence the child | `freopen("/dev/null", "w", stdout)` | also `stderr` |
| before every fork | `fflush(stdout)` | avoids the duplicated text |
| exit code of the shell for "not found" | `127` | convention after failed exec |
