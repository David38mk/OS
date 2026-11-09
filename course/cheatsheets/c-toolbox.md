# C Toolbox Cheat Sheet – the C that the OS exams keep asking for

The C tasks in the first partial are always "processes and threads": read arguments, read a file, start threads or children, collect results, print. The thread and process calls themselves (`fork`, `pthread_create`, `waitpid`, `exec`) are taught in the labs; this sheet covers everything **around** them, the part that quietly costs points: argument parsing, reading files, strings, passing data to threads, error checks, time, building and debugging.

Every snippet exists as a file in `course/cheatsheets/examples/c-toolbox/`. All of them compile with zero warnings and were run:

```
$ gcc -Wall -Wextra -pthread -g NN_name.c -o NN_name && ./NN_name
```

Lines starting with `$` are typed at the prompt; the lines under them are real output. Macedonian task text is quoted where a recipe comes from a real task. For the Python side of the same exams see `python.md`; for the shell side `shell.md`.

---

## 1. Program skeleton

Concept: `main(int argc, char *argv[])` receives the command line. `argc` counts the words including the program name; `argv[0]` is the program name; `argv[argc]` is `NULL`. Everything in `argv` is text.

| Function | Signature | Args | Returns | Notes |
|---|---|---|---|---|
| `atoi` | `int atoi(const char *s)` | text | the number, `0` for garbage | cannot tell `"0"` from `"abc"`; cuts `"12x"` to 12; overflow is undefined. Only for input you trust |
| `strtol` | `long strtol(const char *s, char **end, int base)` | text, where it stopped, base 10 | the number | `*end` points at the first unread char; `errno` is `ERANGE` on overflow |
| `fprintf(stderr, ...)` | | | | usage and errors go to stderr |
| `exit(n)` / `return n` from `main` | | | | `0` success; anything else tells the shell it failed |

Pattern (file `01_args_strtol.c`):

```c
if (argc < 2) {                          // argv[0] is the program name, so 2 = one real argument
    fprintf(stderr, "Upotreba: %s <N>...\n", argv[0]);
    return 1;                            // non-zero = failure, the shell sees it in $?
}
for (int i = 1; i < argc; i++) {
    char *end;                           // strtol stores here where it stopped reading
    errno = 0;
    long v = strtol(argv[i], &end, 10);
    int ok = *argv[i] != '\0'            // something was given
          && *end == '\0'                // the whole text was a number
          && errno == 0                  // no overflow
          && v >= 1 && v <= 1000;        // our own range
```

```
$ ./01_args_strtol 5 12x 99999999999999999999
5                      atoi=5           strtol=5                    OK
12x                    atoi=12          strtol=12                   INVALID
99999999999999999999   atoi=-1          strtol=9223372036854775807  INVALID
```

`atoi` accepted `12x` and returned garbage for the huge number without telling anyone. `strtol` with the three checks (`*s != 0`, `*end == 0`, `errno == 0`) catches all of it. Some exams explicitly say "you may use `atoi`" (2023); then `atoi` is fine, but the check costs 3 lines.

### Sizing arrays: fixed 100, VLA, or malloc

Tasks say "at most 100 files" (2024) or the number depends on `argc` (2022, 2023). Three ways (file `02_array_sizes.c`):

| Way | Code | When | Watch out |
|---|---|---|---|
| fixed | `int fixed[100];` | the task promises a maximum | still check `n > 100` and refuse; 2D: `float nizi[100][100]` is 40 KB, fine as a global |
| VLA (variable length array) | `int vla[n];` | size known only at run time, small | `n` must be at least 1; cannot be global; a huge `n` overflows the stack |
| `malloc` | `int *p = malloc(n * sizeof(int));` | large or must outlive the function | check for `NULL`; `free` it |

```
$ ./02_array_sizes a b c
n=3 sizeof(fixed)=400 sizeof(vla)=12 last=4 4 4
```

The course solutions use a VLA for the thread handles, `pthread_t nitki[num_datoteki];`. That is accepted style, as long as the count is at least 1.

Course files: `course/2026/Labs/Lab2/Z2_S.c` (`strtol` on `-r M`, lines 122 to 135, then `struct Result results[n]` sized by `argc`), `course/2023/Exams/partial_one/Z3.c` (`char ip_adresi[100][16]`: 100 lines of 15 characters plus the `'\0'`).

---

## 2. Reading files

Two families. Pick by what you read:

| | `FILE *` (stdio) | file descriptor (`int`) |
|---|---|---|
| Open | `fopen(name, "r")` | `open(name, O_RDONLY)` |
| Read | `fscanf`, `fgets`, `fgetc`, `fread` | `read(fd, buf, size)` |
| Write | `fprintf`, `fputs`, `fwrite` | `write(fd, buf, len)` |
| Close | `fclose(fp)` | `close(fd)` |
| Failure | `NULL` | `-1` |
| Buffering | yes, in your process | none: one system call per call |
| Good for | numbers, words, lines of **text** (every exam so far) | raw bytes, pipes, files shared with child processes |

### 2.1 `fopen` family

| Function | Signature | Returns | Notes |
|---|---|---|---|
| `fopen` | `FILE *fopen(const char *name, const char *mode)` | `FILE *` or `NULL` | modes `"r"`, `"w"` (empties it), `"a"` |
| `fscanf` | `int fscanf(FILE *fp, const char *fmt, ...)` | number of items converted, or `EOF` | skips whitespace for `%d %lf %s`; **loop while it returns 1**, not `!= EOF` |
| `fgets` | `char *fgets(char *buf, int size, FILE *fp)` | `buf`, or `NULL` at end | reads one line **including `\n`**, at most `size - 1` chars |
| `fputs` / `fprintf` | | | write text; `fputs` adds no newline |
| `fclose` | `int fclose(FILE *fp)` | 0 | flushes output; always close what you opened |
| `strcspn` | `size_t strcspn(const char *s, const char *reject)` | index of first char in `reject` | `line[strcspn(line, "\n")] = '\0'` strips the newline, safe even when there is none |

Conversion letters: `%d` int, `%ld` long, `%f` float, `%lf` **double** (in `scanf`; `printf` uses `%f` for both), `%s` word, `%255s` word of at most 255 chars (always give a width: the buffer must hold width + 1).

**Read every real number from a file into an array** (2024 Z3; file `03_file_numbers.c`):

```c
double niza[MAX];                        // MAX is 100
int n = 0;
while (n < MAX && fscanf(fp, "%lf", &niza[n]) == 1)   // 1 = one value converted; EOF/garbage stops
    n++;
fclose(fp);
```

```
$ ./03_file_numbers
n=4 najgolemo zgolemuvanje=59.75
```

The bound `n < MAX` goes first so a long file cannot write past the array. Keep `n` per array when there are several files (`nizi_size[i]` in the 2024 solution).

**Read lines** (file `04_fgets_lines.c`; the buffer is deliberately 16 bytes to show what a long line does):

```c
while (fgets(line, sizeof(line), fp) != NULL) {   // NULL at end of file
    int had_nl = strchr(line, '\n') != NULL;
    line[strcspn(line, "\n")] = '\0';    // index of the first '\n' (or the end): cut there
```

```
1: [10.0.0.1] len=8
2: [] len=0
3: [192.168.0.255] len=13
4: [posledna bez ne] len=15 (no newline: partial or last)
5: [wline] len=5 (no newline: partial or last)
```

Lines longer than `size - 1` arrive in pieces (4 and 5 are one line). Size the buffer for the longest line the task allows, with room for `\n` and `'\0'`.

**Read words and count occurrences of a word** (2022 Z3; file `06_count_word.c`). `fscanf("%255s")` splits on any whitespace, so line structure does not matter:

```c
char w[MAX_WORD];                        // MAX_WORD is 256
while (fscanf(fp, "%255s", w) == 1) {    // 255 = MAX_WORD - 1: leaves room for the '\0'
    if (strcmp(w, zbor) == 0)            // strcmp == 0 means equal
        found++;
    (*total)++;
}
```

```
OS: 3 od 9 zborovi
nema.txt: -1
```

(`"OS e OS predmet / na FEIT, OS e lab"`: the word is exact, so `"FEIT,"` with its comma is a different word. Say so if the task text is loose about punctuation.) Return `-1` for "cannot open" and let the thread or `main` print the message.

### 2.2 File descriptors

| Function | Signature | Returns | Notes |
|---|---|---|---|
| `open` | `int open(const char *name, int flags, mode_t mode)` | fd or `-1` | flags `O_RDONLY`, `O_WRONLY \| O_CREAT \| O_TRUNC`; `mode` (e.g. `0644`) only when creating |
| `read` | `ssize_t read(int fd, void *buf, size_t n)` | bytes read, `0` at end, `-1` error | may return **less** than `n`; the buffer is **not** terminated |
| `write` | `ssize_t write(int fd, const void *buf, size_t n)` | bytes written or `-1` | takes a length, not a string |
| `close` | `int close(int fd)` | 0 or `-1` | |

File `05_fd_read_write.c`; an 8-byte buffer needs 3 reads for 18 bytes:

```c
ssize_t n;                               // signed: -1 = error, 0 = end of file
while ((n = read(fd, buf, sizeof(buf))) > 0) {
    fwrite(buf, 1, n, stdout);           // buf is NOT terminated: write exactly n bytes
    total += n;
}
```

```
$ ./05_fd_read_write
zdravo od write()
18 bytes in 3 reads
```

---

## 3. Strings

Concept: a C string is a `char` array ending in `'\0'`. There is no length stored; the functions walk to the `'\0'`. Every function that **writes** needs a buffer big enough for the text **plus** the `'\0'`.

| Function | Signature | Returns | Notes |
|---|---|---|---|
| `strlen` | `size_t strlen(const char *s)` | length without `'\0'` | `sizeof` of a char array is its capacity, `strlen` is the text length |
| `strcmp` | `int strcmp(const char *a, const char *b)` | `0` equal, `<0` or `>0` otherwise | the **only** correct "are these equal"; `a == b` compares addresses |
| `strncmp` | `int strncmp(a, b, size_t n)` | like `strcmp`, first `n` chars | prefix test: `strncmp(s, "-r", 2) == 0` |
| `strcpy` | `char *strcpy(char *dst, const char *src)` | `dst` | no bounds check; `dst` must be big enough |
| `strncpy` | `char *strncpy(char *dst, const char *src, size_t n)` | `dst` | copies at most `n`; **adds no `'\0'` when cut**: set `dst[n-1] = '\0'` yourself |
| `strcat` | `char *strcat(char *dst, const char *src)` | `dst` | appends; total must still fit |
| `strchr` / `strrchr` | `char *strchr(const char *s, int c)` | pointer to first / last `c`, or `NULL` | `strrchr(path, '.')` finds the extension |
| `strstr` | `char *strstr(const char *s, const char *sub)` | pointer to first `sub`, or `NULL` | substring search |
| `snprintf` | `int snprintf(char *buf, size_t size, const char *fmt, ...)` | length it **wanted** to write | never writes more than `size`; `result >= size` means truncated. Prefer it to `sprintf` |
| `sscanf` | `int sscanf(const char *s, const char *fmt, ...)` | number of items converted | `sscanf(ip, "%d.%d.%d.%d", &a, &b, &c, &d) == 4` validates the shape (2023 Z3) |
| `strtok` | `char *strtok(char *s, const char *delims)` | next token or `NULL` | **modifies `s`**, keeps hidden state, not thread-safe |
| `strtok_r` | `char *strtok_r(char *s, const char *delims, char **save)` | next token or `NULL` | same, state in `save`: use it in threads |

File `08_str_funcs.c`:

```
OS-lab 6 0 1 0                       <- strcat, strlen, strcmp equal, "OS" < "PS", strncmp prefix equal
[abc]                                <- strncpy cut at 3 chars, terminated by hand
ext=.c at=12 slash=home/david/OS/lab.c
need=11 got=[datoteka:] truncated=yes   <- snprintf into 10 bytes: wanted 11
in.txt_out                           <- snprintf(big, sizeof(big), "%s_out", "in.txt")
```

### `strtok` in detail

File `07_strtok.c`:

```c
char original[] = "ls -l  -a\tfile.txt";   // an ARRAY: writable. char *p = "..." would crash strtok
char copy[sizeof(original)];
strcpy(copy, original);                  // strtok writes '\0' into the string: work on a copy
for (char *t = strtok(copy, " \t"); t != NULL; t = strtok(NULL, " \t"))   // NULL = continue same string
    printf("token [%s]\n", t);
```

```
token [ls]   token [-l]   token [-a]   token [file.txt]
copy is now "ls" (cut at the first token), original is still "ls -l  -a<TAB>file.txt"
strtok_r counted 4 tokens
ip parts 192 168 1 20
sscanf of bad text returns 2
```

Three rules: (1) the first call gets the string, later calls get `NULL`; (2) the string is **destroyed** (each separator becomes `'\0'`), so copy it first if you still need it; (3) the position lives in one hidden global, so two threads tokenising at once corrupt each other. Exam threads that tokenise their own line must use `strtok_r`, each with its own copy and its own `save` pointer. File `14_strtok_r_threads.c`:

```c
struct Linija { char tekst[64]; int zborovi; };   // each thread owns its text
void *broi(void *arg) {
    struct Linija *l = arg;
    char *save;                                   // private position
    for (char *t = strtok_r(l->tekst, " ", &save); t != NULL; t = strtok_r(NULL, " ", &save))
        l->zborovi++;
    return NULL;
}
```

```
linija 0: 3 zborovi
linija 1: 6 zborovi
```

Beyond this course: `strdup` and `getline` for dynamic strings.

---

## 4. Structs for threads and children

Concept: `pthread_create` gives the thread **one** `void *`. To pass several values (word + file name, array + length + result), put them in a `struct` and pass its address. The same struct can carry the result back, which removes the `malloc` and cast dance of returning a pointer.

Rules: pass `&z[i]` (the address of array element `i`; each thread gets its own struct). Never pass `&i` of the loop variable (it changes while the thread starts) and never `(void *) i` followed by a dereference (the index becomes an **address**; for `i = 0` that is `NULL`, see section 7). Keep the array alive until after `pthread_join`. Start from `memset(z, 0, sizeof(z))` so counters and results are 0, not garbage. Write the result into the thread's own struct field: no `malloc`, no shared variable, no race.

File `09_struct_threads.c`:

```c
struct Zadacha {
    const char *ime;     // input: which file / currency
    double niza[MAX];    // input: the values
    int n;
    double rezultat;     // output: written by the thread, read by main after join
};

void *najgolem_rast(void *arg) {
    struct Zadacha *z = arg;                    // void * back to the real type
    ...
    return NULL;                                // result already lives in the struct
}
// in main:
pthread_create(&t[i], NULL, najgolem_rast, &z[i]);   // address of element i
```

```
$ ./09_struct_threads
BTC: 24.0
ETH: 48.0
EOS: 72.0
```

The alternative used in the 2022 to 2024 solutions: the thread returns a `malloc`ed result, `main` reads it with `pthread_join(t, &res)` and calls `free(res)`. It works; the struct cannot leak. A child made by `fork` gets a **copy** of every variable at that moment and can only send back an exit status (`_exit(n)`, read with `waitpid`), a file, or a pipe.

Course files: `course/2025/Exercises/threads/NitkiPrvaVezba.c` (an `int redenBrNitki[5]` array, one slot per thread, `&redenBrNitki[i]` passed: the correct pattern), `course/2022/Exams/partial_one/Z3.c` (file name and shared `global_zbor`), `course/2024/Exams/partial_one/Z3.c` (arrays of arrays `float nizi[100][100]` plus `nizi_size[100]`).

---

## 5. `errno`, `perror`, `strerror`

Concept: a failing system or library call returns a marker (`NULL`, `-1`) and stores a reason number in the global `errno`. **Check the return value first**; `errno` is only meaningful right after a failure.

| Call | Signature | Notes |
|---|---|---|
| `perror(prefix)` | `void perror(const char *s)` | prints `prefix: reason` on stderr |
| `strerror(n)` | `char *strerror(int n)` | the reason text for number `n`, to build your own message |
| `errno` | `int`, from `<errno.h>` | save it at once (`int e = errno;`): `printf` and `perror` can change it |

Where the failure shows up:

| Call | Failure value | Reason in |
|---|---|---|
| `fopen` | `NULL` | `errno` |
| `open`, `read`, `write`, `close` | `-1` | `errno` |
| `fork` | `-1` (`0` is the child, `> 0` the parent) | `errno` |
| `malloc` | `NULL` | `errno` |
| `waitpid` / `wait` | `-1` | `errno` |
| `pthread_create`, `pthread_join` | **non-zero return value** | the **return value itself**, not `errno` |
| `execl*`, `execv*` | only returns when it **failed**, always `-1` | `errno` |

File `10_errno.c`:

```
$ ./10_errno
fopen: No such file or directory
open: Permission denied
errno=2 (No such file or directory)
errno after a success: 13        <- a success does NOT clear errno: never test errno alone
child status 7
```

`_exit(127)` after `exec` ends the child without flushing stdio buffers copied from the parent (`exit` would print them a second time); 127 is the shell's "command not found" convention. See `course/2026/Labs/Lab2/Z2_S.c` lines 54 to 76.

---

## 6. Time

| Function | Signature | Returns | Resolution | Notes |
|---|---|---|---|---|
| `time` | `time_t time(time_t *t)` | seconds since 1970 | 1 s | wall clock; use `time(NULL)`; also the usual `srand(time(NULL))` seed |
| `difftime` | `double difftime(time_t a, time_t b)` | `a - b` in seconds | 1 s | |
| `clock_gettime` | `int clock_gettime(clockid_t id, struct timespec *ts)` | 0 | nanoseconds | `CLOCK_MONOTONIC`: never jumps back when the date is changed |
| `sleep` | `unsigned sleep(unsigned s)` | remaining seconds | 1 s | whole seconds |
| `usleep` | `int usleep(useconds_t us)` | 0 | microseconds | old but short to write |
| `nanosleep` | `int nanosleep(const struct timespec *req, struct timespec *rem)` | 0 | nanoseconds | `{sec, nsec}`; `rem` gets the time left when a signal interrupts it |

`time()` is too coarse for a timeout. It only ticks once per second: if you start at `10.99 s` and check `time() - start >= 2`, "2 seconds" is true after **1.01** s of real time. The comment in `course/2026/Labs/Lab2/Z2_S.c` (lines 35 to 38) says exactly this, and the solution measures with `CLOCK_MONOTONIC` instead.

File `11_time.c`:

```c
static double now_s(void) {                         // seconds with fractions, never jumps backwards
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + ts.tv_nsec / 1e9;
}
...
struct timespec pauza = {0, 300000000L};            // 0 s + 300,000,000 ns = 0.3 s
nanosleep(&pauza, NULL);
```

```
$ ./11_time
monotonic: 0.5 s
time():    0 s                  <- 0 here; 1 if a second boundary fell in between
after sleep(1): 1.5 s
timeout reached after 2.0 s
```

A timeout loop checks the clock every 100 ms instead of sleeping the whole `T`, so fast results are noticed at once and the limit is exact to about 0.1 s (`Z2_S.c` line 111: `{0, 100000000L}`, with a comment why `sleep(1)` would be wrong; the simpler `Z2.c` uses `usleep(100000)` for the same reason). Waiting for a child without blocking: `waitpid(pid, &st, WNOHANG)` returns `0` while it still runs.

---

## 7. Build and debug

| Flag | Meaning |
|---|---|
| `-Wall -Wextra` | turn on the warnings that find real bugs (do this **always**) |
| `-pthread` | needed whenever you use `pthread_*`; without it the link fails or behaves oddly |
| `-g` | keep names and line numbers in the binary, for `gdb` |
| `-o name` | output name (default `a.out`) |

Compile: `gcc -Wall -Wextra -pthread -g Z3.c -o Z3`. **A warning is a bug report. Do not hand in code that has one.** Warnings that the repo's own files produce (real output):

| Warning | Meaning | Where it appeared |
|---|---|---|
| `'ip_adresi' undeclared` (error) | a global is used above the line that declares it: C reads top to bottom | `2023/.../Z3.c` line 21, `2024/.../Z3.c` lines 27 and 28 |
| `cast to pointer from integer of different size [-Wint-to-pointer-cast]` | a number was passed as `(void *)i` | `2024/.../Z3.c` line 75 |
| `control reaches end of non-void function [-Wreturn-type]` | a thread function declared `void *` has no `return` | `2025/Exercises/threads/Kpromeni.c`, `1000br.c` |
| `implicit declaration of function 'exit'` | missing `#include <stdlib.h>` | `2025/Labs/Lab2/Z3.c` line 19 |
| `missing sentinel in function call` | `exec*` argument list not ended with `(char *)NULL` | (shown by a test file; see pitfalls) |
| `function returns address of local variable` | returning a pointer to a local array | (shown by a test file; see pitfalls) |
| `unused variable` | harmless, but remove it so real warnings stay visible | several |

(On newer GCC 14 an implicit declaration is an error, not a warning.)

### `gdb`, six commands

Compile with `-g`. Session on `03_file_numbers` (the commands are given with `-ex`, but typed interactively they are the same):

```
$ gdb ./03_file_numbers
(gdb) break 22              # stop at line 22 (or: break main, break function_name)
(gdb) run                   # start the program (run arg1 arg2 for arguments)
Breakpoint 1, main () at 03_file_numbers.c:22
22	        if (niza[i] - niza[i - 1] > max)
(gdb) print n               # value of a variable
$1 = 4
(gdb) print niza[0]@n       # n elements starting at niza[0]
$2 = {100.5, 90.25, 150, 120.75}
(gdb) next                  # execute one line, do not enter functions (step enters them)
(gdb) continue              # run until the next breakpoint
(gdb) bt                    # backtrace: where am I and who called me (use after a crash)
```

Crash example (a version of `2024/.../Z3.c` with the declarations moved up so it compiles). The program ended silently with exit status 0 in the parent; in the child, `set follow-fork-mode child` makes gdb follow it:

```
Thread 2.2 "z24g" received signal SIGSEGV, Segmentation fault.
0x00005555555552eb in najgolemo_zgolemuvanje (num=0x0) at z24.c:26
26	    int broj_nitka = *(int*)num;
```

`num=0x0`: the thread received a null pointer because `(void *)nitki_broj[i]` turned the index 0 into an address. The `bt` plus `print` workflow found the cause in two commands.

`strace` (system calls the program makes), one line:

```
$ strace -f -e trace=openat,write ./05_fd_read_write 2>&1 | grep -E "fd.txt|write\("
openat(AT_FDCWD, "fd.txt", O_WRONLY|O_CREAT|O_TRUNC, 0644) = 3
write(3, "zdravo od write()\n", 18)     = 18
openat(AT_FDCWD, "fd.txt", O_RDONLY)    = 3
```

`-f` follows children and threads; `-e trace=...` limits the output. It answers "did my program even open the file, and what did the system say".

---

## 8. Exam recipes

The shape is always: arguments -> read -> create workers -> collect -> print.

**C1. N threads, one per file, count something, then a percentage** (2022 Z3). Task: „За секоја датотека во која се прави пребарување, се креира посебна нитка … Откако ќе завршат сите нитки, главната нитка на екран печати, за секоја датотека одделно, колку процентуално се појавил зборот.“
1. `argc` check; word = `argv[1]`, files = `argv[2..]`; fewer than 2 files: read them with `scanf("%255s", ...)` (width!).
2. One `struct` per thread: file name, word, count (section 4). Create all threads first, then join all.
3. In the thread: `fopen` (check `NULL`), `fscanf("%255s")` loop, `strcmp`, store the count in the struct (section 2.1, file `06_count_word.c`).
4. After the joins: `total = sum`, percent `= 100.0 * count / total` only when `total > 0` (division by zero), print with `%.2f%%` (a literal percent sign is `%%`).

**C2. A thread per line of a file** (2023 Z3). Task: „Главната нитка треба да креира онолку нитки колку што има IP адреси во датотеката.“
1. Read lines into `char ip[100][16]` with `fscanf("%15s", ...)` and a `broj < 100` guard (section 2.1).
2. Each thread gets its **index** by address (`&idx[i]`), or better its own struct holding the string.
3. Parse with `sscanf(ip, "%d.%d.%d.%d", ...) == 4`, then range-check each part to 0..255.
4. Return a `long long` (the count can exceed `int`: 256^4 is 4.3 billion): in a struct field, or a `malloc`ed result that `main` frees.

**C3. Arrays of values, a child process, threads inside the child** (2024 Z3). Task: „… главниот процес креира дете процес на кој му ги испраќа овие низи … Дете процесот најпрво треба да креира онолку нитки колку што има низи.“
1. Fixed `float nizi[100][100]`, `int nizi_size[100]` as **globals declared above the thread function**.
2. Read each file with `fscanf("%f")` into row `i` (section 2.1); the parent does this **before** `fork`, so the child owns a copy of the arrays automatically (this is what "sends them" means).
3. `fork`; in the child: one thread per row, pass `&idx[i]` (never `(void *)i`), join, print; in the parent: `wait(NULL)`.
4. BONUS (print through `echo`): build the command with `snprintf(cmd, sizeof(cmd), "echo %s %.2f", ...)` and run it with `system(cmd)`.

**C4. Run a command N times / ping many hosts** (2025 Lab 2 Z3, 2026 Lab 2 Z2). Task: „Родител процесот најпрво од тастатура го вчитува бројот колку пати ќе се изврши наредбата. … секој еден од тие N процеси ја извршува наредбата.“
1. `argc < 2`: usage on stderr, `return 1`. Read `N` with `scanf("%d", &N) == 1` and check `N >= 1`.
2. Loop `N` times: `fflush(stdout)`, `fork`, child: `execvp(argv[1], &argv[1]); perror(argv[1]); _exit(127);`.
3. Parent: `N` times `wait(NULL)` (or `waitpid` with a timeout loop and `kill(pid, SIGKILL)`, section 6).
4. Print the final line exactly as the task says: „Zavrshuva process 0“, with `\n`.

---

## 9. Pitfalls

Format: symptom -> cause -> fix.

1. **Crash or odd values after typing a long word.** `scanf("%s", w)` has no limit; the word writes past `w`. Test with `char w[4]` and the input `abcdefghijklmnop`: Ubuntu's gcc stops the program with `*** stack smashing detected ***` (exit status 134); without the protection it is `Segmentation fault` (139). Fix: a width, `scanf("%255s", w)` with `char w[256]`, or `fgets`.
2. **`if (a == b)` is false for equal words, or `argv[1] == "-r"` never matches.** `==` compares addresses, not text. gcc says `comparison with string literal results in unspecified behavior`. Fix: `strcmp(a, b) == 0` (file `12_pitfalls_safe.c`).
3. **Segmentation fault in `strtok`.** `char *p = "a b";` points into read-only memory and `strtok` writes into it (verified: exit status 139 = 128 + signal 11). Fix: `char p[] = "a b";` or copy with `strcpy` into an array.
4. **`exec` fails strangely or crashes.** The argument list of `execl`/`execlp` must end with `(char *)NULL`, and the `argv` array given to `execv`/`execvp` needs a final `NULL` entry too. Without it the function reads past the list: undefined behaviour. gcc warns `missing sentinel in function call`. Fix: always add `(char *)NULL`.
5. **Wrong results when comparing a length with a number.** `strlen`, `sizeof` and `sizeof`-based values are `size_t` (unsigned): `-1 < strlen(s)` is **false** because `-1` becomes a huge unsigned number (file `12_pitfalls_safe.c` prints `0`). Fix: compare as `int` after a cast `(int)strlen(s)`, or keep both sides signed. `-Wextra` warns `comparison of integer expressions of different signedness`.
6. **Counter starts at a random value.** A local `int count;` is not zero (gcc: `'c' is used uninitialized`). Fix: `int count = 0;`; globals and `memset`-ed structs start at 0.
7. **Garbage text printed from a function.** `char a[8] = "hi"; return a;` returns the address of a local array that dies at the `}` (gcc: `function returns address of local variable`). Fix: `static char a[8]`, a buffer passed in by the caller, or `malloc` (and `free` later).
8. **Output appears late, out of order, or twice.** `stdout` is buffered: line-by-line on a terminal, but in big blocks when piped or redirected, while `stderr` is not buffered. And `fork` copies the unflushed buffer into the child. File `13_buffering.c`, run to a pipe:
   ```
   $ ./13_buffering 2>&1 | cat
   2. stderr, bez baferiranje              <- stderr first, although printed second
   1. stdout, ima newline
   3. bez newline i bez fflush (dete)      <- the child flushed a copy of lines 1 and 3
   1. stdout, ima newline
   3. bez newline i bez fflush 4. kraj     <- the parent printed them again
   ```
   Fix: end every message with `\n`, `fflush(stdout)` **before** `fork`, and use `_exit` in a child after a failed `exec`. In `2025/Labs/Lab2/Z3.c` the messages have no `\n` (`printf("Pogresen broj na argumenti")`), so they run together with the shell prompt.
9. **A failed `exec` looks like a success.** After `execvp(...)` the next line only runs when exec **failed**. `2025/Labs/Lab2/Z3.c` then calls `exit(0)`: the parent sees status 0 for a command that never ran, and (without `<stdlib.h>`) the compiler warns. Fix: `perror`, then `_exit(127)`.
10. **All threads print the same number, or a number that is not theirs.** `pthread_create(..., &i)` passes the address of the loop variable, which changes under the thread. Or `(void *) i` then `*(int *)num`: the index 0 becomes `NULL` and the thread crashes (the 2024 crash in section 7). Fix: an array of values or structs, one per thread, pass `&arr[i]` (section 4).
11. **`pthread_join` on a thread that was never created.** If `pthread_create` fails and the loop goes on, the matching `pthread_t` is uninitialised (`2022/.../Z3.c` builds a "dummy result" in that branch but never uses it). Fix: on failure, stop creating, join only the first `i` threads, or exit with a message.
12. **"Undeclared" on the grader.** A global or function used above its declaration (the 2023 and 2024 solutions). Fix: declare globals and structs at the top of the file.
13. **`int` overflow in a count.** 256^4 addresses wrap around an `int` at 2.1 billion. Fix: `long long` and `%lld`.
14. **Errors invisible to the shell.** Falling off the end of `main` returns 0. Fix: `return 1;` after every error message, printed to `stderr`.

Beyond this course: `valgrind` and `-fsanitize=address` find memory bugs automatically.

---

## 10. Quick reference

| I need to ... | Code |
|---|---|
| check argument count | `if (argc < 2) { fprintf(stderr, "Upotreba: %s ...\n", argv[0]); return 1; }` |
| number from text, safely | `long v = strtol(s, &end, 10);` then `*s && !*end && errno == 0` |
| open a file or stop | `FILE *fp = fopen(n, "r"); if (!fp) { perror(n); return 1; }` |
| all numbers into an array | `while (n < MAX && fscanf(fp, "%lf", &a[n]) == 1) n++;` |
| all words | `while (fscanf(fp, "%255s", w) == 1)` with `char w[256]` |
| text equal | `strcmp(a, b) == 0` |
| split on spaces | `strtok(copyOfString, " ")`, then `strtok(NULL, " ")`; threads: `strtok_r` |
| data to a thread | `struct` in an array, `pthread_create(&t[i], NULL, f, &z[i])` |
| child runs a command | `execvp(argv[1], &argv[1]); perror("exec"); _exit(127);` |
| wait without blocking | `waitpid(pid, &st, WNOHANG)` (`0` = still running) |
| fine-grained time | `clock_gettime(CLOCK_MONOTONIC, &ts)` |
| compile | `gcc -Wall -Wextra -pthread -g x.c -o x` |
| debug | `gdb ./x`: `break`, `run`, `bt`, `next`, `print`, `continue` |
| flush before fork | `fflush(stdout);` |

The examples, by section: `01`, `02` section 1; `03` to `06` section 2; `07`, `08`, `14` section 3; `09` section 4; `10` section 5; `11` section 6; `12`, `13` section 9 (pitfalls).
