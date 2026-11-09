# Pipes Cheat Sheet – study guide for the OS labs and exams

Everything here was compiled with `gcc -Wall -Wextra -pthread` and run on Linux (Ubuntu 24.04, glibc 2.39). Each code block is a trimmed copy of a full file in `examples/pipes/`; the path is given under it. Output lines are real output. Macedonian task quotes are kept as in the course files.

---

## 0. What a pipe is and why you need one

`fork()` gives the child a **copy** of the parent's memory. After that, a variable changed in one process is not changed in the other. A **pipe** is the simplest way to send bytes between related processes: the kernel keeps a small buffer, one process writes into one end, another reads from the other end.

- **File descriptor (fd):** a small integer (0, 1, 2, 3, ...) that names an open file, pipe or terminal. 0 is stdin, 1 is stdout, 2 is stderr.
- **EOF (end of file):** `read()` returns 0.
- A pipe is one direction only. For two directions, make two pipes.

```
   parent process                                  child process
  +------------------+     kernel buffer      +------------------+
  | write(fd[1],...) |--->[ a b c d e ... ]-->| read(fd[0],...)  |
  +------------------+   (about 64 KiB)       +------------------+
         fd[1] = write end            fd[0] = read end
```

What the course tasks ask for, in one table:

| Task shape | Sheet section |
|---|---|
| Parent sends text or numbers to child (or back) | 2, 3 |
| Redirect stdin/stdout so `printf`/`scanf` use a pipe or file | 4 |
| Build `cmd1 \| cmd2` (Lab 6 Z2) | 5 |
| Collect results from N children | 3, 7 |

---

## 1. The pipe and `fork()`: who holds which end

`pipe(fd)` creates **both** ends in the calling process. `fork()` then copies the whole fd table, so after the fork **both processes hold both ends**:

```
 after pipe(fd)     after fork()                after closing the unused ends
 +--------+        +--------+  +--------+       +--------+            +--------+
 | parent |        | parent |  | child  |       | parent |            | child  |
 | fd[0]  |        | fd[0]  |  | fd[0]  |       | fd[0] X|            | fd[0] -+--+
 | fd[1]  |        | fd[1]  |  | fd[1]  |       | fd[1] -+--+         | fd[1] X|  |
 +--------+        +--------+  +--------+       +--------+  |         +--------+  |
                                                            v                     |
                                                     ======[ pipe ]===============+
                                                  (X = closed)  data flows parent -> child
```

**Rule: every process closes the ends it does not use, as early as possible.** Two reasons, both explained in section 2:
the reader only sees EOF when *no* write end is open anywhere, and the writer is killed by a signal if no read end is open.

---

## 2. The four calls

| Function | Signature | Args | Returns | Errors (`errno`) |
|---|---|---|---|---|
| `pipe` | `int pipe(int fd[2])` | `fd[0]` is filled with the read end, `fd[1]` with the write end | 0 / -1 | `EMFILE`, `ENFILE` (too many open files), `EFAULT` |
| `read` | `ssize_t read(int fd, void *buf, size_t n)` | up to `n` bytes into `buf` | bytes read; **0 = EOF**; -1 | `EBADF`, `EINTR`, `EAGAIN` (non-blocking only) |
| `write` | `ssize_t write(int fd, const void *buf, size_t n)` | `n` bytes from `buf` | bytes written; -1 | `EPIPE` (no reader; the signal `SIGPIPE` arrives first), `EBADF`, `EINTR` |
| `close` | `int close(int fd)` | | 0 / -1 | `EBADF` |

Also needed: `fork()` (pid in parent, 0 in child, -1 on error), `wait(NULL)` / `waitpid(pid, NULL, 0)` from `<sys/wait.h>`.

### How `read` behaves

| Pipe state | `read` does |
|---|---|
| has data | returns what is there, **at most** `n`, **maybe fewer** than you asked |
| empty, some write end still open (anywhere) | **blocks** (sleeps) |
| empty, **all** write ends closed | returns **0** (EOF) |

`write` blocks when the buffer is full (64 KiB on Linux) until a reader makes room. Writes of up to `PIPE_BUF` bytes (4096 on Linux) are **atomic**: they are never mixed with another process's write.

`read` returns bytes, not a string: add the `'\0'` yourself if you want to `printf("%s")`.

### Minimal pattern: parent writes, child reads until EOF

```c
int fd[2];
if (pipe(fd) == -1) { perror("pipe"); return 1; }
pid_t pid = fork();
if (pid == 0) {                    // child: reader
    close(fd[1]);                  // child never writes
    char buf[64]; ssize_t n;
    while ((n = read(fd[0], buf, sizeof buf - 1)) > 0) {
        buf[n] = '\0';
        printf("child got %zd bytes: \"%s\"\n", n, buf);
    }
    printf("child: read returned %zd (EOF)\n", n);
    close(fd[0]); return 0;
}
close(fd[0]);                      // parent never reads
write(fd[1], msg, strlen(msg));
close(fd[1]);                      // last write end closed -> child sees EOF
wait(NULL);
```

Full file: `examples/pipes/01_pipe_basic.c`

```
child got 19 bytes: "zdravo od roditelot"
child: read returned 0 (EOF)
```

### The hang: one write end left open

`examples/pipes/02_hang_open_write_end.c` is the same program, but with an argument the parent **skips `close(fd[1])`**. The parent then waits for the child, the child waits for EOF, and EOF never comes because the parent itself still holds a write end. Nobody is ever woken: a deadlock.

```
$ timeout 2 ./02_hang_open_write_end          # correct version
child got 5 bytes
child: EOF, leaving
$ timeout 2 ./02_hang_open_write_end bug      # parent forgot close(fd[1])
$ echo $?
124                                           # timeout killed it after 2 s: it was hanging
```

`timeout 2 cmd` kills `cmd` after 2 seconds and exits with 124. Use it whenever you test code that might hang.

The same happens with a leftover **copy of a write end in another child** (section 3, N children) or in the **parent of a pipeline** (section 5).

---

## 3. Parent and child protocols

### 3.1 Text: write, read in a loop

Never assume one `write` of N bytes arrives as one `read`. Always loop until `read` returns 0 (text) or until you got a known number of bytes (binary). `Lab6/Z1.c` does it correctly:

```c
while ((procitano = read(pipe1[0], buffer, Buffer)) > 0)
    write(pipe2[1], buffer, procitano);     // pass on exactly what was read
close(pipe1[0]);
close(pipe2[1]);                            // closing is what lets the next stage reach EOF
```

Note `write(..., procitano)`, not `strlen(buffer)`: the buffer is not null-terminated.

### 3.2 Binary: ints and structs

`write(fd, &x, sizeof x)` copies the raw bytes of `x`; the reader needs `read(fd, &x, sizeof x)` with the **same type**. Works for `int`, `double`, arrays and structs with fixed-size members. Send a count first when the number of items is not fixed.

```c
struct rec { int id; double value; char name[16]; };   // fixed size, no pointers
...
write(fd[1], &n, sizeof n);                 // parent: count first
for (int i = 0; i < n; i++) { struct rec r = {...}; write(fd[1], &r, sizeof r); }
...
read(fd[0], &n, sizeof n);                  // child: same order, same sizes
for (int i = 0; i < n; i++) {
    if (read(fd[0], &r, sizeof r) != sizeof r) { puts("short read"); break; }
    ...
}
```

Full file: `examples/pipes/03_binary_records.c`

```
child: 3 records
child: id=1 value=1.50 name=item0
child: id=2 value=3.00 name=item1
child: id=3 value=4.50 name=item2
```

Never put a `char *` (or any pointer) inside such a struct: see pitfall 7.

### 3.3 Both directions: two pipes

One pipe = one direction. If both sides must talk, make `down` and `up` and give each process one end of each:

```
 parent --write down[1]--> down[0]-- child
 parent <--read  up[0]---- up[1] --- child         each side closes its 2 unused ends
```

```c
int down[2], up[2];  pipe(down);  pipe(up);
if (fork() == 0) {
    close(down[1]); close(up[0]);                    // child: reads down, writes up
    int x; read(down[0], &x, sizeof x);
    long sq = (long)x * x;  write(up[1], &sq, sizeof sq);
    return 0;
}
close(down[0]); close(up[1]);                        // parent: writes down, reads up
write(down[1], &x, sizeof x);  read(up[0], &answer, sizeof answer);
```

Full file: `examples/pipes/04_two_pipes.c` (prints `parent: 12 squared is 144`).

### 3.4 N children, results back to the parent

| Design | Parent reads | Good for | Watch out |
|---|---|---|---|
| **One shared pipe**, each child writes one fixed-size record that contains its number | loop `read` until 0, records arrive in finishing order | few results, order irrelevant | parent must `close(fd[1])` before reading; records <= `PIPE_BUF` stay unmixed |
| **One pipe per child** (`int fd[N][2]`) | `read(fd[i][0], ...)` for `i = 0..N-1` | results needed in child order | each child must close the read ends of older pipes; parent closes every write end |

One shared pipe, from `examples/pipes/05_n_children_one_pipe.c`:

```c
struct result { int child; long sum; };
for (int i = 0; i < N; i++) {
    if (fork() == 0) {
        close(fd[0]);
        struct result r = { i, 0 };
        for (int k = 1; k <= (i + 1) * 100; k++) r.sum += k;
        write(fd[1], &r, sizeof r);             // one write = one atomic record
        close(fd[1]); return 0;
    }
}
close(fd[1]);                                   // BEFORE the read loop
while (read(fd[0], &r, sizeof r) == sizeof r)
    printf("parent: child %d sum=%ld\n", r.child, r.sum);
```

```
parent: child 0 sum=5050
parent: child 1 sum=20100
parent: child 2 sum=45150
parent: child 3 sum=80200
```

One pipe per child: `examples/pipes/06_pipe_per_child.c`. It also shows the other direction for free: the parent built `data[N][LEN]` **before** `fork()`, so every child already has a copy. See recipe R5.

---

## 4. `dup2` and redirection

`scanf`, `printf`, `fgets` always use fd 0 (stdin) and fd 1 (stdout). To make them use a pipe or a file, **change what fd 0 / fd 1 point to** with `dup2`.

| Function | Signature | Does | Returns | Errors |
|---|---|---|---|---|
| `dup2` | `int dup2(int oldfd, int newfd)` | closes `newfd` if open, then makes `newfd` another name for what `oldfd` points to | `newfd` / -1 | `EBADF`, `EMFILE`, `EINTR` |
| `dup` | `int dup(int oldfd)` | duplicate onto the lowest free fd (used to save the real stdout) | new fd / -1 | `EBADF`, `EMFILE` |
| `open` | `int open(const char *path, int flags, mode_t mode)` | opens a file | fd / -1 | `ENOENT`, `EACCES`, ... |

Argument order: `dup2(from, to)`. **The thing you have is first, the fd number you want to take over is second.**

```
  dup2(fd[1], STDOUT_FILENO);          before                 after
                                    fd 1 --> screen         fd 1 --\
                                    fd[1] --> pipe write    fd[1] --+--> pipe write end
  then close(fd[1]) so only fd 1 remains
```

### 4.1 Through a pipe

```c
// child
close(fd[1]);
dup2(fd[0], STDIN_FILENO);  close(fd[0]);       // fd 0 now IS the read end
scanf("%d %d", &a, &b);
// parent
close(fd[0]);
dup2(fd[1], STDOUT_FILENO); close(fd[1]);       // fd 1 now IS the write end
printf("20 22\n");
fflush(stdout);                                  // see below
close(STDOUT_FILENO);                            // last write end gone -> child gets EOF
```

Full file: `examples/pipes/07_dup2_stdout_stdin.c`, output `child: 20 + 22 = 42`.

**`fflush` rule.** `printf` writes into a buffer inside your program, not straight to fd 1. On a terminal the buffer is flushed at each `\n`; into a pipe or file it is flushed only when full or at exit. So call `fflush(stdout)` **before** you `dup2` over fd 1, **before** you `exec`, and when the other side must see the data now. Unflushed text is lost on `exec` and copied by `fork`.

### 4.2 To and from a file

`> file` is `open(O_WRONLY|O_CREAT|O_TRUNC, 0644)` then `dup2(fd, 1)`. `< file` is `open(O_RDONLY)` then `dup2(fd, 0)`.

```c
int in  = open("/tmp/08_in.txt",  O_RDONLY);
int out = open("/tmp/08_out.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
int screen = dup(STDOUT_FILENO);               // keep the real stdout
dup2(in,  STDIN_FILENO);  close(in);           // like:  < /tmp/08_in.txt
dup2(out, STDOUT_FILENO); close(out);          // like:  > /tmp/08_out.txt
scanf("%31s %d", word, &n);
printf("word=%s twice=%d\n", word, 2 * n);     // lands in the file
fflush(stdout);
dup2(screen, STDOUT_FILENO);                   // back to the screen
```

Full file: `examples/pipes/08_redirect_file.c`.

| Flag | Meaning |
|---|---|
| `O_WRONLY` / `O_RDONLY` / `O_RDWR` | write only / read only / both |
| `O_CREAT` | create if missing (then the mode argument is needed) |
| `O_TRUNC` | empty the file first (like `>`); without it old bytes stay (like writing over text) |
| `O_APPEND` | writes go to the end (like `>>`) |

Mode `0644` = owner read+write, others read. (`0666` in the course files works too; the real mode is `mode & ~umask`.)

**Close the original fd after `dup2`.** After `dup2(fd[1], 1)` there are two names for the write end. Leave the second open and the reader never gets EOF (section 5).

### Course file using this: `2025/Labs/Lab5/pipes/Z1.c`

Despite the folder name it uses **plain files** `pipe1.txt` and `pipe2.txt` as "pipes" and three separate runs (`./a.out 1`, then `2`, then `3`). Process 1 reads words until `0`, writes them to `pipe1.txt` through stdout, and to the screen through a saved copy of stdout (Задача 2):

```c
int ekran = dup(1);      // zacuvaj originalen stdout (ekran)
dup2(fd, 1);             // stdout -> pipe1.txt
close(fd);
...
printf("%s ", s);  fflush(stdout);   // goes to the file
dprintf(ekran, "%s ", s);            // goes to the screen
```

Process 2 copies `pipe1.txt` to `pipe2.txt` and the screen with `read`/`write(1, ...)`/`write(ekran, ...)`. Process 3 counts the spaces. Run on the box, it works:

```
$ printf 'ana ima kuce 0\n' | ./z1 1
ana ima kuce
$ ./z1 2
ana ima kuce
$ ./z1 3
Procesot broj 3 izbroi 3 prazni mesta
```

Teaching note: `scanf("%99s", s)` is not checked. If stdin ends before the user types `0`, `scanf` returns EOF (-1), `s` keeps its old value and the `while (1)` never ends. Check `scanf(...) == 1`. Files are not pipes: process 2 only works because process 1 already finished.

---

## 5. Building a shell pipeline by hand

`ls -1 | wc -l`: the shell starts two processes and connects the stdout of the first to the stdin of the second. Recipe: **one pipe, two forks, two `dup2`, close all four fd copies, `exec`, parent waits for both.**

```
          pipe
 ls ----> [ ======= ] ----> wc
 fd 1                       fd 0
 (dup2(fd[1],1))            (dup2(fd[0],0))       after dup2 each child closes fd[0] AND fd[1]
 parent: close(fd[0]); close(fd[1]); waitpid x2
```

```c
pid_t p1 = fork();
if (p1 == 0) {                          // left side: stdout -> pipe
    dup2(fd[1], STDOUT_FILENO);
    close(fd[0]); close(fd[1]);         // both originals, or wc never sees EOF
    execlp("ls", "ls", "-1", (char *)NULL);
    perror("execlp ls"); _exit(127);
}
pid_t p2 = fork();
if (p2 == 0) {                          // right side: stdin <- pipe
    dup2(fd[0], STDIN_FILENO);
    close(fd[0]); close(fd[1]);
    execlp("wc", "wc", "-l", (char *)NULL);
    perror("execlp wc"); _exit(127);
}
close(fd[0]); close(fd[1]);             // the parent holds both ends too
waitpid(p1, NULL, 0);  waitpid(p2, NULL, 0);
```

Full file: `examples/pipes/09_pipeline_ls_wc.c`. It prints the number of entries in the current directory.

| Function | Signature | Notes |
|---|---|---|
| `execvp` | `int execvp(const char *file, char *const argv[])` | searches `PATH`; `argv[0]` is the program name, **the array must end with `NULL`**; returns only on failure (-1, `errno` set), so the next line is the error path |
| `execlp` | `int execlp(const char *file, const char *arg0, ..., (char *)NULL)` | same, arguments listed one by one |
| `waitpid` | `pid_t waitpid(pid_t pid, int *status, int options)` | `0` options = block until that child ends; needs `<sys/wait.h>` |
| `_exit` | `void _exit(int)` | use after a failed `exec` in a child: skips flushing stdio copies inherited from the parent |

**Why `exec` keeps the redirection:** `exec` replaces the program but keeps the fd table. `ls` simply writes to fd 1 and does not know it is a pipe.

### Why leftover fds break it

`wc` stops when `read` on its stdin returns 0, that is when **every** write end is closed. Count the holders:

| Holder of write end `fd[1]` | Must close it | If forgotten |
|---|---|---|
| `ls` child, original `fd[1]` after `dup2` | yes, right after `dup2` | harmless, it exits with `ls` |
| `wc` child | yes, it never writes | **`wc` waits for itself: hang** |
| parent | yes, before `waitpid` | **hang: `wc` never sees EOF, parent waits for `wc`** |

### The generic version: commands from `argv`

`examples/pipes/10_pipeline_argv_file.c` splits `argv` at a `"|"` argument and also redirects the last command's stdout into a file (`cmd1 | cmd2 > file`):

```
$ ./10_pipeline_argv_file ls -1 / "|" sort -r
$ head -3 /tmp/10_out.txt
var
usr
tmp
```

The core is the second child:

```c
int out = open("/tmp/10_out.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
dup2(fd[0], STDIN_FILENO);          // stdin <- pipe
dup2(out, STDOUT_FILENO);           // stdout -> file
close(fd[0]); close(fd[1]); close(out);
execvp(argv[bar + 1], argv + bar + 1);
```

For plain `cmd > file` or `cmd < file` drop the pipe: fork, `open`, `dup2`, `close`, `exec`, parent `wait`.

### Course file using this: `2025/Labs/Lab6/Z2.c`

A **three-stage** pipeline `p1 | p2 | p3` with two pipes. Each child asks the user for a command name, builds `args` = command name plus the program's own `argv[1..]`, `dup2`s its ends, and `execvp`s:

```c
if ((pid1 = fork()) == 0) {                // stage 1: only writes to pipe1
    close(pipe1[0]); close(pipe2[0]); close(pipe2[1]);
    ...
    dup2(pipe1[1], 1);  close(pipe1[1]);
    execvp(command, args);
} else if ((pid2 = fork()) == 0) {         // stage 2: reads pipe1, writes pipe2
    close(pipe1[1]); close(pipe2[0]);
    ...
    dup2(pipe1[0], 0);  dup2(pipe2[1], 1);
    close(pipe1[0]);    close(pipe2[1]);
```

Stage 3 closes both ends of `pipe1` and `pipe2[1]`, reads `pipe2[0]`. The parent closes **all four** fds, then three `waitpid`s. The close pattern is correct: every process closes every end it does not use.

Real problems, found by compiling it:

- `pintf(...)` after `execvp` is a typo. gcc only warns (`implicit declaration`), the **linker fails**: `undefined reference to 'pintf'`. The file as shipped does not build.
- `waitpid` needs `#include <sys/wait.h>`, which is commented out.
- All three children call `fgets(…, stdin)` on the same stdin. Typed at a keyboard, each gets one line. With the input all at once (`printf 'ls\nsort\nsort\n' | ./z2`) child 1's stdio reads **all** the text into its buffer; child 3 finds nothing and prints `Neuspesno isvrsena comanda execvp`. See pitfall 6.
- With the typos fixed and the commands entered one at a time, `./z2 -r` with `ls`, `sort`, `sort` works (`ls -r | sort -r | sort -r`).

---

## 6. Pipes vs shared memory vs files

| | Pipe | Shared memory (System V / mmap) | Plain file |
|---|---|---|---|
| Direction | one way (two pipes for both) | any way, both read and write | any way |
| Who can use it | processes that inherit the fds (parent, children) | any process that knows the key / file name | any process that knows the name |
| Size | 64 KiB buffer, writer blocks when full | fixed at creation | grows |
| Synchronisation | **built in**: reader sleeps until data, EOF on close | **none**, you add it (parent `wait()`, flag byte) | none |
| Data type | a stream of bytes, read once and consumed | memory you index like an array, stays until removed | bytes, stays |
| Cleanup | disappears with the last process | System V segment stays until `IPC_RMID` | file stays |

Rule of thumb: a **stream** that must not be lost, one reader: pipe. A **table** both sides read and write at random positions: shared memory (see `shared-memory.md`).

Beyond this course: FIFOs (`mkfifo`, pipes with a file name, usable by unrelated processes); POSIX/System V message queues.

---

## 7. Exam recipes

**R1. Lab 5 Z1, three commands over files** (`2025/Labs/Lab5/pipes/Z1.c`). `./a.out N` picks the role. Role 1: stdin words to `pipe1.txt` (stdout redirected) and the screen (`dup(1)` saved first). Role 2: copy `pipe1.txt` to `pipe2.txt` and the screen. Role 3: count `' '` in `pipe2.txt`. Teaches: `open` flags, `dup`/`dup2`/`close`, `dprintf(fd, ...)`, `read` loop. Run the roles one after another.

**R2. Lab 6 Z1, three roles over two pipes** (`2025/Labs/Lab6/Z1.c`, skeleton `2025/Exercises/pipes/PIPES.c` only creates the two pipes). Role 1 reads lines until an empty line and writes to `pipe1`; role 2 copies `pipe1` to `pipe2`; role 3 counts spaces in what arrives on `pipe2`. The close pattern per role is right. But as shipped each `./a.out N` call creates **its own** pipes, so the three runs are not connected: role 1 alone dies with `SIGPIPE` (shell status 141, it writes to a pipe whose read end it closed itself), role 3 alone sees EOF at once and prints `Prazni mesta se 0`, partly by luck, because `int count;` is never initialised. To make the three roles talk, `fork()` three children from **one** `main` after both `pipe()` calls, as in `09_pipeline_ls_wc.c`, and write `int count = 0;`.

**R3. Lab 6 Z2, `p1 | p2 | p3`** (section 5). Skeleton: 2 pipes, 3 forks, `dup2` per role, then every child closes all pipe fds it still holds, `execvp`, parent closes all 4 and waits for 3. Fix `pintf` and add `<sys/wait.h>` first.

**R4. 2020 partial two, Z3_2: compare a word from stdin with a word from a file** (`2020/Exams/partial_two/Z3/Z3_2.c`; `Z3.c`, `Z3_1.c`, `Z3_3.c` in that folder are kernel-module code, not this topic).

```c
if (dete > 0) {                          // parent
    close(pipe1[0]);
    dup2(pipe1[1], 1);                   // stdout -> pipe
    scanf("%s", tekst);                  // word from the keyboard
    printf("%s", tekst);                 // goes into the pipe
} else {                                 // child
    close(pipe1[1]);
    int fd = open("dat.txt", O_RDONLY, 0666);
    dup2(pipe1[0], 0);  scanf("%s", mem);    // word from the pipe
    dup2(fd, 0);        scanf("%s", vnes);   // word from dat.txt
    ... strcmp(mem, vnes) -> "Isti se" / "Ne se"
```

The idea is right: parent reads the keyboard word and sends it through the pipe via stdout; child reads the first word from the pipe through stdin, then re-points stdin at the file and reads the second word. Two real traps, found by running it (pitfall 6):

```
$ echo hello > dat.txt
$ echo hello | ./Z3_2
Ne se                       # expected "Isti se"
```

1. The parent prints `"%s"` with **no newline**. The child's `scanf` can only finish the word when it sees a delimiter, so it reads on and hits EOF. glibc (2.28 and later, including this box) remembers that EOF on the `stdin` object, so the **second** `scanf`, even though `fd 0` now points at a file, returns EOF at once and `vnes` stays empty. Fix: `printf("%s\n", tekst)` or `clearerr(stdin);` before the second `dup2`. With the `\n` the same test prints `Isti se`.
2. If more text than one word sits in the pipe, the first `scanf` has already pulled all of it into stdio's buffer, and the second `scanf` may read from that buffer instead of the file. `examples/pipes/11_stdio_buffer_eof.c` shows both.
   Also: the parent never waits for the child, and `dat.txt` must already exist.

Safer recipe: use `read(fd, buf, n)` on the raw fds, or `fgets` on `fdopen`ed streams, not `scanf` on a re-pointed `stdin`.

**R5. "Send arrays from parent to child"** (shape of `2024/Exams/partial_one/Z3.c`). The task says the parent "sends" the arrays to the child. After `fork()` the child **already has a copy** of every array the parent filled before the fork, so nothing needs to travel:

```c
// parent: read files into nizi[i][j] and nizi_size[i]   -- before fork
dete = fork();
if (dete == 0) { /* child: threads read nizi[...] directly */ }
else           { wait(NULL); }
```

A pipe is needed only for what happens **after** the fork (parent builds data later, or the data must go to a program started with `exec`, which does not keep memory). Use section 3.2 for that: send the count, then the values.
Teaching notes on this file as shipped: `nizi` and `nizi_size` are declared **after** the thread function that uses them (compile error `'nizi_size' undeclared`; move the two globals above it), and `(void *)nitki_broj[i]` casts an `int` to a pointer while the thread does `*(int *)num`. Pass `&nitki_broj[i]` instead. The child prints the result itself, which is why no return path is needed.

**R6. "Get N results back from N children".** The reverse direction needs a channel: a pipe (section 3.4) or shared memory (`shared-memory.md`, recipe R5). Exit codes are no substitute: a child's exit status holds only 0 to 255.

---

## 8. Pitfalls

1. **The reader hangs forever.** Cause: some write end is still open, usually the parent's own or a sibling child's copy. Fix: every process closes unused ends right after `fork`; the parent closes its write end before reading or waiting. Test with `timeout 2`.
2. **Deadlock when a process both reads and holds its own write end.** Cause: it reads before writing, or reads its own pipe while waiting for EOF that only itself can cause. Fix: close the end you do not use before the first `read`; with two pipes always close 2 of 4 ends per process.
3. **`write` kills the program, shell status 141.** Cause: no read end is open any more, kernel sends `SIGPIPE`. Fix: do not close the read ends too early, or the reader exits early on purpose; `signal(SIGPIPE, SIG_IGN)` makes `write` return -1 with `EPIPE` instead.
4. **Wrong end used.** `fd[0]` is for reading, `fd[1]` for writing (think: 0 is stdin, 1 is stdout). Writing to `fd[0]` fails with `EBADF`.
5. **`dup2` arguments swapped.** `dup2(0, fd[0])` redirects the pipe, not stdin. Rule: `dup2(what_I_have, the_number_I_want)`.
6. **stdio buffers mixed with raw fds.** `scanf`/`fgets`/`printf` keep their own buffer. (a) `scanf` and `fgets` read **ahead**: they pull a whole chunk (up to 4096 bytes) out of the fd and keep it in the buffer. If you then `dup2` something else onto fd 0, that data is gone from the pipe and still sits in the old buffer; the new source is not read until the buffer is empty. (b) After EOF, glibc keeps `stdin` in an EOF state until `clearerr(stdin)`. (c) Several processes sharing one stdin, as in Lab 6 Z2, let the first one swallow everything typed. (d) `printf` text is lost on `exec` and sent twice if unflushed at `fork`. Fix: `fflush` before `fork`/`dup2`/`exec`; for protocols use `read`/`write` on the fd only; end text with `\n`.
7. **Writing a struct that contains a pointer.** `write(fd, &b, sizeof b)` copies the pointer **value**, not what it points to, and an address is meaningless in the other process.
   ```
   parent: text lives at 0x7ffcc177bd24
   child: id=7 name pointer=0x7ffcc177bd24 (not dereferenced)
   ```
   It looks identical here only because `fork` copies the address space; after `exec` or between unrelated processes it is garbage. Fix: `char name[16]` inside the struct (`examples/pipes/12_struct_with_pointer.c`).
8. **One `read` per `write`.** A `read` may return fewer bytes than asked, or two writes' worth at once. Fix: loop until 0; for binary records loop until you have `sizeof` bytes (or use fixed-size records and compare against `sizeof`).
9. **Treating the buffer as a string.** `read` does not add `'\0'`. Use the returned count: `buf[n] = '\0'`, or `write(fd, buf, n)`.
10. **Uninitialised or unchecked values.** `int count;` then `count++` (Lab 6 Z1), `scanf` result ignored (Lab 5 Z1), `pipe()`/`fork()`/`open()` unchecked. Initialise and test the return values.
11. **`exec` fails silently.** Code after `execvp` runs only on failure: print with `perror` and `_exit(127)`; do not let the failed child continue running the parent's code.

---

## 9. Quick reference

| Task | Code |
|---|---|
| Create a pipe | `int fd[2]; pipe(fd);` read end `fd[0]`, write end `fd[1]` |
| Child reads, parent writes | child `close(fd[1])`; parent `close(fd[0])` |
| Signal "no more data" | writer `close(fd[1])`; reader loops `while ((n = read(...)) > 0)` |
| Both directions | two pipes, each process closes 2 of the 4 ends |
| stdout into pipe | `dup2(fd[1], STDOUT_FILENO); close(fd[0]); close(fd[1]);` |
| stdin from pipe | `dup2(fd[0], STDIN_FILENO); close(fd[0]); close(fd[1]);` |
| `> file` | `open(f, O_WRONLY\|O_CREAT\|O_TRUNC, 0644)`, `dup2(fd, 1)`, `close(fd)` |
| `< file` | `open(f, O_RDONLY)`, `dup2(fd, 0)`, `close(fd)` |
| Save and restore stdout | `int s = dup(1); ... fflush(stdout); dup2(s, 1);` |
| `cmd1 \| cmd2` | pipe, fork x2, `dup2` + close all 2 ends in each child, `execvp`, parent closes both and `waitpid` x2 |
| Binary record | `write(fd, &r, sizeof r)` / `read(fd, &r, sizeof r) == sizeof r`, no pointers inside |
| N results back | one shared pipe of fixed-size records, or `fd[N][2]` |
| Test for hangs | `timeout 2 ./prog; echo $?` (124 = it hung) |
| Atomic write limit | `PIPE_BUF` = 4096 bytes on Linux |
| Flush before | `fork`, `dup2` over stdout, `exec` |
| Exec never returns on success | line after `execvp` is the error path: `perror`, `_exit(127)` |
| Beyond this course | FIFOs (`mkfifo`), message queues |
