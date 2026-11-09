# Threads Cheat Sheet – study guide for the OS labs and exams

How a C program does several things at once inside one process with POSIX threads (`pthread_create`, `pthread_join`, `pthread_mutex_*`). Read sections 0 to 6 in order once, use 7 as a pattern library, 9 before an exam, 10 when something misbehaves. Processes (`fork`, `exec`, `wait`) are in `processes.md`; this sheet only repeats what threads need.

Every code block marked with a file name is a real program in `examples/threads/`. Compile and run in the Linux box (the `oslab` container):

```bash
$ gcc -Wall -Wextra -pthread examples/threads/01_create_join.c -o /tmp/a && /tmp/a
```

All of them compile with zero warnings. Output below is real. Task quotes are in Macedonian, exactly as in the course files.

---

## 0. What a thread is and why we use it

A **thread** is a separate line of execution inside one process. All threads of a process see the **same memory** (globals, heap, open files); each has only its own stack and registers. So threads can work on one array together without copying anything, and they can also step on each other (section 5). Typical exam use: "one thread per file / per array / per IP", then the main thread adds up the answers.

```
 ONE PROCESS (one pid)
 +-----------------------------------------------------------+
 |  code      globals       heap (malloc)    open files      |   shared by all threads
 |  int niza[1000];   int *res = malloc(..)                  |
 +-----------------------------------------------------------+
 |  main thread   |  thread 1      |  thread 2      |  ...   |
 |  own stack     |  own stack     |  own stack     |        |   private: locals, args
 |  own registers |  own registers |  own registers |        |
 +-----------------------------------------------------------+
```

---

## 1. The thread model

| | Process (`fork`) | Thread (`pthread_create`) |
|---|---|---|
| Creation cost | larger: copies the address space (copy-on-write) | small: new stack only |
| Memory | **own copy**, nothing shared | **shared** globals and heap |
| Talking to each other | pipe, shared memory, file, exit code | plain variables (with a lock when written) |
| One crashes | the others live on | the whole process dies (one bad pointer kills all threads) |
| Can `exec`? | yes, its job | no useful way: exec replaces the **whole** process (section 8) |
| Own pid | yes | no, same pid, but a `pthread_t` id |

What is private and what is shared, in code terms:

| Variable | Where | Shared? |
|---|---|---|
| global or `static` variable | data segment | yes, all threads |
| memory from `malloc` | heap | yes, whoever has the pointer |
| local variable inside a thread function | that thread's stack | no |
| local variable of `main` | main's stack | the **address** can be passed to threads: then yes, and it must outlive them |

---

## 2. pthread_create

Headers `<pthread.h>`; compile with `-pthread` (section 6).

| | |
|---|---|
| Signature | `int pthread_create(pthread_t *tid, const pthread_attr_t *attr, void *(*start)(void *), void *arg)` |
| `tid` | where the new thread's id is stored (you need it for `join`) |
| `attr` | `NULL` = default attributes (always in this course) |
| `start` | the function the thread runs: `void *f(void *arg)` |
| `arg` | one pointer handed to `start` |
| Returns | `0` on success, an **error number** otherwise (`EAGAIN` too many threads). It does **not** set `errno` |

| Function | Signature | Returns |
|---|---|---|
| `pthread_self` | `pthread_t pthread_self(void)` | id of the calling thread, never fails |
| `pthread_equal` | `int pthread_equal(pthread_t a, pthread_t b)` | non-zero if same thread |

Minimal complete pattern, `examples/threads/01_create_join.c`:

```c
void *work(void *arg)
{
    int i = *(int *)arg;            // read the index BEFORE main can change it
    printf("thread %d running\n", i);
    return NULL;
}

int main(void)
{
    pthread_t tid[4];
    int idx[4];                     // one slot per thread: each thread gets its own address
    for (int i = 0; i < 4; i++) {
        idx[i] = i;
        int rc = pthread_create(&tid[i], NULL, work, &idx[i]);
        if (rc != 0) { fprintf(stderr, "pthread_create: error %d\n", rc); return 1; }
    }
    for (int i = 0; i < 4; i++)
        pthread_join(tid[i], NULL);
    printf("all joined\n");
    return 0;
}
```

```
thread 0 running
thread 1 running
thread 2 running
thread 3 running
all joined
```

(The order of the "running" lines is up to the scheduler; do not rely on it.)

The course files all use this shape: an `int indexi[N]` array and `(void*)&indexi[i]`, see `course/2025/Exercises/threads/NitkiPrvaVezba.c`:

```c
for(i=0;i<5;i++){
  redenBrNitki[i]=i;
  int rc = pthread_create(&nitki[i],NULL,pechati,(void*)&redenBrNitki[i]);
  if(rc){ printf("Greska"); return -1; }
}
```

### Passing arguments: the `&i` trap

The thread reads `*arg` **later**, when it gets scheduled. If you passed `&i` of the loop variable, by then `i` has moved on, and every thread reads the same variable. `02_arg_trap.c`:

```
$ ./02_arg_trap            # &idx[i]: one slot per thread
thread got 0
thread got 1
thread got 2
thread got 3
$ ./02_arg_trap bad        # &i: all threads share the one variable
thread got 4
thread got 4
thread got 4
thread got 4
```

Rule: **every thread needs its own argument object that lives until it has read it.** `Kpromeni.c` in the course folder makes the same mistake with a global `k` that main changes in the loop (`&k`) while threads read it.

| Need | Do |
|---|---|
| one int (index) | array `int idx[N]`, pass `&idx[i]` |
| several values | one `struct` per thread in an array, pass `&jobs[i]` (`03_struct_args.c`) |
| a result back | a field of that struct, or a slot in a global array (section 3) |
| one small int, quick | `(void *)(intptr_t)i` in `create`, `(int)(intptr_t)arg` in the thread (`<stdint.h>`). Works, but only for one small integer. The 2024 exam file casts with `(void *)nitki_broj[i]` in `create` and then dereferences `*(int*)num` in the thread: pick one style, never mix |

`03_struct_args.c`:

```c
struct Job { int id; const char *word; int result; };
void *work(void *arg)
{
    struct Job *j = arg;
    printf("job %d got word '%s'\n", j->id, j->word);
    j->result = j->id * 10;
    return NULL;
}
/* main: struct Job jobs[3]; ...  pthread_create(&tid[i], NULL, work, &jobs[i]); */
```

The `jobs` array lives in `main`, and `main` joins before returning, so the pointers stay valid.

---

## 3. pthread_join and pthread_exit

| Function | Signature | Meaning | Returns |
|---|---|---|---|
| `pthread_join` | `int pthread_join(pthread_t tid, void **retval)` | block until thread `tid` ends; its return value goes into `*retval` (pass `NULL` to ignore) | `0` or error number (`ESRCH`, `EINVAL`, `EDEADLK`) |
| `pthread_exit` | `void pthread_exit(void *retval)` | end the calling thread with a value | does not return |
| `pthread_detach` | `int pthread_detach(pthread_t tid)` | thread cleans itself up, cannot be joined | `0` |

`return x;` at the end of the thread function is the same as `pthread_exit(x)`. `join` is the thread version of `wait`: you must join every thread you created (otherwise its resources stay allocated, and the **main thread can end first**, below). Beyond this course: detached threads in practice (one line: `pthread_detach(tid)` and never join).

### Getting a result back

Two ways, both used in the course (`04_return_value.c`):

```c
void *square(void *arg)
{
    int i = *(int *)arg;
    int *res = malloc(sizeof *res); // way 1: never return the address of a local variable
    if (res == NULL) return NULL;
    *res = i * i;
    slot[i] = i + 100;              // way 2: write into this thread's cell of a global array
    return res;                     // same as pthread_exit(res)
}
/* main: */
void *ret;
pthread_join(tid[i], &ret);         // join stores the return value in ret
if (ret != NULL) { printf("%d\n", *(int *)ret); free(ret); }   // the joiner frees it
```

```
thread 0 returned 0, slot[0]=100
thread 1 returned 1, slot[1]=101
thread 2 returned 4, slot[2]=102
```

| Way | Good when | Careful |
|---|---|---|
| `malloc`'d result, `return res`, `join(&ret)` | each thread produces one value; exam files 2022/2023/2024 | `free` it after reading; check for `NULL` |
| slot in a global array `slot[i]` | results of different types, or you want them without casting | read it only after `join` |
| never `return &local` | `local` is on the thread's stack, which is gone after it ends | |

Join all threads in a loop; they can finish in any order but `join` simply waits for each in turn:

```c
for (int i = 0; i < N; i++) pthread_join(tid[i], NULL);
```

### When main returns, the others die

`return` from `main` (or `exit`) ends the **process**, and all threads with it. `05_main_exits.c`:

```
$ ./05_main_exits
main returns now                  # the thread never printed: killed with the process
$ ./05_main_exits x               # with the join
slow thread finished
main returns now
```

If main must finish but the threads should keep running, call `pthread_exit(NULL)` in `main` instead of `return`. In exams: always `join`.

---

## 4. Sharing data

Threads share globals, so the simplest design in this course is: **input global and read-only, one output slot per thread, main combines.** That needs no lock at all, because nobody writes a cell that someone else reads or writes at the same time. `06_slots.c`:

```c
int data[T * PER];                  // shared, only READ by the threads
long partial[T];                    // thread i writes ONLY partial[i]
void *sum_part(void *arg)
{
    int i = *(int *)arg;
    long s = 0;
    for (int j = i * PER; j < (i + 1) * PER; j++) s += data[j];
    partial[i] = s;
    return NULL;
}
/* main fills data BEFORE creating threads, adds partial[i] AFTER joining thread i */
```

```
total = 4500 (expected 4500)
```

| Situation | Lock needed? |
|---|---|
| all threads only read the same data | no |
| each thread writes its own cell/field, main reads after `join` | no (join is the synchronisation) |
| two or more threads write the **same** variable (counter, total, one array cell) | **yes** (section 5) |
| main writes while threads read | avoid: fill before `create`, or lock |

`course/2025/Exercises/threads/NitkiNiza.c` is the model: 10 threads, each owns the slice `pole[i*100 .. i*100+99]` and writes `maxN[i]`.

---

## 5. Race conditions and mutexes

`counter++` looks like one step but is three: read, add, write back. If two threads interleave, one update is lost. That is a **race condition**: the result depends on timing. `Kpromeni.c` and `1000br.c` in `course/2025/Exercises/threads/` update shared variables (`promeni++`, the "found" total in the bonus) without a lock. `07_race.c`: 4 threads each add 1 a million times.

```
$ ./07_race nolock
nolock: counter = 1551652 (expected 4000000)
$ ./07_race nolock
nolock: counter = 1295322 (expected 4000000)
$ ./07_race nolock
nolock: counter = 1280898 (expected 4000000)
$ ./07_race
lock  : counter = 4000000 (expected 4000000)
```

Different wrong value every run, correct with the mutex. A **mutex** (mutual exclusion lock) lets one thread at a time into a protected region (the **critical section**); the others wait at `lock`.

| Function | Signature | Meaning | Returns |
|---|---|---|---|
| `PTHREAD_MUTEX_INITIALIZER` | `pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;` | initialise a global/static mutex | |
| `pthread_mutex_init` | `int pthread_mutex_init(pthread_mutex_t *m, const pthread_mutexattr_t *attr)` | initialise at run time (`attr` = `NULL`) | `0` or error number |
| `pthread_mutex_lock` | `int pthread_mutex_lock(pthread_mutex_t *m)` | wait until free, then own it | `0` or error number |
| `pthread_mutex_unlock` | `int pthread_mutex_unlock(pthread_mutex_t *m)` | release it | `0` or error number |
| `pthread_mutex_destroy` | `int pthread_mutex_destroy(pthread_mutex_t *m)` | free it, only when nobody holds or waits | `0` or error number |

Pattern: every thread that touches the variable does `lock`, touch, `unlock`, around **the same mutex**.

```c
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
...
pthread_mutex_lock(&lock);
counter++;                          // the critical section: as short as possible
pthread_mutex_unlock(&lock);
```

Keep the critical section small: do private work outside, only the shared update inside (`08_mutex_init.c`, mutex inside a struct, shows `init`/`destroy`):

```c
struct Shared { pthread_mutex_t m; int total; };
...
int mine = i % 7;                   // private work stays OUTSIDE the lock
pthread_mutex_lock(&s->m);
s->total += mine;                   // only the shared update is inside
pthread_mutex_unlock(&s->m);
/* main: pthread_mutex_init(&s.m, NULL); ... join all ...; pthread_mutex_destroy(&s.m); */
```

```
total = 8991 (expected 8991)
```

`course/2025/Labs/Lab5/threads/Z1.c` uses a global `fileMutex` to guard the shared read offset (`globalOffset`) and the `finished` flag, and holds it across the check and the update: check-then-act must be **one** critical section. It also unlocks before every `break`; a forgotten unlock on one path freezes all other threads.

**Deadlock**, in one paragraph: if thread A holds mutex 1 and waits for mutex 2 while thread B holds mutex 2 and waits for mutex 1, both wait forever. Same with locking the same non-recursive mutex twice. Avoid it by taking several locks always in the same order, and by not calling anything that may lock again while you hold one.

Beyond this course: condition variables, barriers, read-write locks, semaphores, `pthread_mutex_trylock`.

---

## 6. Compiling

```bash
$ gcc -Wall -Wextra -pthread prog.c -o prog
```

Use `-pthread` (not only `-lpthread`): it sets the compile flags **and** links the library. Without it you may get at link time:

```
/usr/bin/ld: prog.c:(.text+0x31): undefined reference to `pthread_create'
collect2: error: ld returned 1 exit status
```

On the current glibc (2.34 and later) the pthread functions are in the main C library, so a program may link without the flag; an older lab machine or the grader may not. Always compile with `-pthread`. `-Wall -Wextra` finds most mistakes in this sheet's pitfalls: a thread function that forgets `return`, unused variables, int-to-pointer casts. Include `<pthread.h>` (a typo like `<phtread.h>` is a fatal error, as in `course/2025/Exercises/processes/Knaredbi.c`).

---

## 7. Patterns

Each is a full program in `examples/threads/`.

### 7.1 One thread per file, count a word (2022 Z3 shape, `09_word_count.c`)

Global `count[NF]`, thread `i` opens file `i`, counts, stores `count[i]`. Main joins all, **then** computes percentages (they need the grand total).

```c
void *count_word(void *arg)
{
    int i = *(int *)arg;
    FILE *f = fopen(name[i], "r");
    if (f == NULL) { perror(name[i]); return NULL; }     // count[i] stays 0
    char w[256];
    while (fscanf(f, "%255s", w) == 1)
        if (strcmp(w, word) == 0) count[i]++;
    fclose(f);
    printf("%s: %d times\n", name[i], count[i]);
    return NULL;
}
/* main, after all joins: */
printf("%s: %.2f%%\n", name[i], total ? 100.0 * count[i] / total : 0.0);
```

```
/tmp/t09_a.txt: 3 times
/tmp/t09_b.txt: 0 times
/tmp/t09_c.txt: 2 times
/tmp/t09_a.txt: 60.00%
/tmp/t09_b.txt: 0.00%
/tmp/t09_c.txt: 40.00%
```

Guard `total == 0` (division by zero). The course file `course/2022/Exams/partial_one/Z3.c` returns a `malloc`'d int instead of using a slot; both are fine.

### 7.2 One thread per array, find a maximum (2024 Z3 shape, `10_max_per_array.c`)

Thread `a` scans `arr[a]` and returns the largest difference between neighbours as a heap `float`:

```c
float arr[NA][LEN] = { ... };       // declared BEFORE the thread function that uses it
void *max_rise(void *arg)
{
    int a = *(int *)arg;
    float *res = malloc(sizeof *res);
    if (res == NULL) return NULL;
    *res = 0.0f;
    for (int i = 1; i < LEN; i++)
        if (arr[a][i] - arr[a][i - 1] > *res) *res = arr[a][i] - arr[a][i - 1];
    return res;
}
```

```
Najgolemo zgolemuvanje za BTC e: 4.00
Najgolemo zgolemuvanje za ETH e: 0.00
Najgolemo zgolemuvanje za EOS e: 3.00
```

The same idea, with slots instead of `malloc`: `course/2025/Exercises/threads/NitkiNiza.c` (`maxN[i]`, then main takes the max of the maxima).

### 7.3 One thread per IP, count, main sums (2023 Z3 shape, `11_ip_count.c`)

The task: from each given IP count how many addresses can be generated up to `255.255.255.255`. An IP is a 32-bit number `a*2^24 + b*2^16 + c*2^8 + d`, so the count is `0xFFFFFFFF - value + 1`; no four nested loops needed (the course solution loops 256^4 times at worst). Use `long long` for the sum.

```c
long long here = ((long long)p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
*res = 0xFFFFFFFFLL - here + 1;
```

```
255.255.255.250 -> 6
255.255.254.0 -> 512
255.255.255.255 -> 1
Vkupno mogni IP adresi: 519
```

Main joins in a loop, `total += *(long long *)ret; free(ret);`. Note on `course/2023/Exams/partial_one/Z3.c`: the global `ip_adresi` is declared **after** the thread function that uses it, so the file as saved does not compile; move the declaration up.

### 7.4 One thread per child process, waiting for it (`12_waiter_threads.c`)

Rule: **fork all children first, create the threads afterwards.** Each thread blocks in `waitpid(kid[i], &st, 0)` for its own child and records the time:

```c
void *waiter(void *arg)
{
    int i = *(int *)arg;
    double t0 = now_s();
    int st;
    waitpid(kid[i], &st, 0);        // blocks until THIS child ends, no polling, no sleep
    secs[i] = now_s() - t0;
    code[i] = WIFEXITED(st) ? WEXITSTATUS(st) : -1;
    return NULL;
}
```

```
child 0 (pid 5858): exit code 0, waited 1.0 s
child 1 (pid 5859): exit code 0, waited 2.0 s
child 2 (pid 5860): exit code 0, waited 0.0 s
```

Three versions exist in the course; they differ in **how a thread waits**:

| File | How it waits | Result |
|---|---|---|
| `course/2025/Exercises/processes/NizaNaredbi.c` | loop `waitpid(pid, NULL, WNOHANG)` + `sleep(1)` | polling: wakes every second, time is a whole number of seconds, always rounded up by the polling step |
| `12_waiter_threads.c` (here) | one blocking `waitpid(pid, &st, 0)` | exact, no wakeups; fine when the parent has no timeout |
| `course/2026/Labs/Lab2/Z2_threads.c` | `waitid(P_PID, pid, &si, WEXITED \| WNOWAIT)`, then `waitpid` under a mutex | blocking, **and** the child stays a zombie until the thread reaps it while holding the lock |

Why `WNOWAIT` in `Z2_threads.c`: the main thread may `kill(pid, SIGKILL)` at the timeout. If a thread had already reaped the child, that pid is free and the system could give it to an unrelated new process, and `kill` would hit a stranger. With `WNOWAIT` the child stays a zombie (the pid stays reserved) until the thread reaps it **under the same mutex** main holds while it kills; so `kill` always hits either our live child or our zombie. That is the reason, not speed. If your task has no kill/timeout, the plain blocking `waitpid` above is enough.

### 7.5 Main prints totals after the joins

Anything that needs **all** results (percentages, grand totals, "the largest of the maxima") is computed by main after the join loop, never inside a thread. Sections 7.1 to 7.3 all end this way.

---

## 8. Processes and threads together

Many exam tasks say "работи со процеси и нитки". Rules:

1. **Fork first, then create threads.** `fork` copies only the calling thread. If other threads exist at that moment, they do not exist in the child, but memory is copied as it was, including any mutex one of them was holding: nobody in the child can ever unlock it, so the child can hang. That is why `Z2_threads.c` forks **all** children before creating any thread.
2. **A child may create threads of its own.** After `fork`, the child is a normal single-threaded process and can `pthread_create` as much as it wants. This is the 2024 exam shape.
3. **Memory goes parent to child, not back.** The child gets a copy of every array the parent filled before `fork`. Its results stay in its copy. `13_fork_then_threads.c`:

```c
for (a ...) for (i ...) arr[a][i] = ...;     // parent fills BEFORE fork
pid_t p = fork();
if (p == 0) {                                // child: threads are created AFTER the fork
    /* create one thread per array, join, print */
    fflush(stdout); _exit(0);
}
waitpid(p, NULL, 0);
printf("parent: best[0] = %d (the child's result never came back)\n", best[0]);
```

```
child : array 0 largest = 4
child : array 1 largest = 6
child : array 2 largest = 6
parent: best[0] = 0 (the child's result never came back)
```

4. **Threads cannot `exec` usefully.** `exec` replaces the **whole process**: all other threads and `main` vanish. `14_exec_in_thread.c`:

```
thread: calling execlp, nothing after it in this process will ever run
echo: I am now the whole process
```

   The line after `pthread_join` in main never ran. If a thread must run a program, it calls `fork()` (the child then has only that one thread) and `exec` in the child, or `system()`.
5. Threads **share** the process: a thread calling `exit()` ends the whole program, as `exit(0)` does in `course/2025/Labs/Lab3/threads/Lab3.c` when a thread finds the number first (that is intentional there).

---

## 9. Exam recipes

### Recipe 1. One thread per file (2022 partial one, Z3)

> „За секоја датотека во која се прави пребарување, се креира посебна нитка која го прави пребарувањето … Откако ќе завршат сите нитки, главната нитка на екран печати, за секоја датотека одделно, колку процентуално се појавил зборот во таа датотека.“

1. Fill `word` and `files[]` from `argv` or the keyboard (processes.md section 9, recipe 4).
2. Per file, one thread: create with `&idx[i]`, or a pointer to the name.
3. Thread: `fopen` (on failure: message, return 0), loop `fscanf("%255s")` and `strcmp`, print "file: count", return the count (slot or `malloc`).
4. Main: join all, sum, then print `100.0 * count[i] / total` (guard `total == 0`).

Pattern 7.1, `examples/threads/09_word_count.c`.

### Recipe 2. One thread per IP (2023 partial one, Z3)

> „Главната нитка треба да креира онолку нитки колку што има IP адреси во датотеката. Секоја нитка, пресметува колку можни IP адреси може да генерира … На крај, главниот процес(нитка) печати на екран, колку вкупно (од сите нитки) може да се генерираат IP адреси.“

1. `argc < 2` → message, return 1. `fopen(argv[1])`, check `NULL`.
2. Read lines with `fscanf("%15s")` into a global `char ip[100][16]` (declare it **before** the thread function), count them.
3. One thread per line; thread parses with `sscanf("%d.%d.%d.%d")` (check `== 4`), returns a `malloc`'d `long long`.
4. Main joins, adds up (`long long`), `free`s, prints the total.

Pattern 7.3, `examples/threads/11_ip_count.c`.

### Recipe 3. Processes and threads: crypto values (2024 partial one, Z3)

> „Главниот процес … креира дете процес на кој му ги испраќа овие низи … Дете процесот најпрво треба да креира онолку нитки колку што има низи (крипто-валути) … да го најдат најголемото зголемување помеѓу две соседни вредности.“

1. Parent: for each argument open the file, read floats into `nizi[i][j]` (global, at most 100), store `nizi_size[i]`. Declare the globals before the thread function.
2. `fork`; in the child create one thread per array (pass the **address** of an index array), thread returns a heap `float`.
3. Child joins, prints "name: increase", `_exit(0)`. Parent `waitpid`.
4. Why it works: section 8, point 3. Bonus (print with `echo`): `execlp("echo", "echo", text, (char *)NULL)` at the very end of the child.

Known problems in `course/2024/Exams/partial_one/Z3.c` as saved: the global arrays are declared after the thread function (does not compile); `create` passes `(void*)nitki_broj[i]` (an int) but the thread does `*(int*)num` (crash): pass `&nitki_broj[i]`.

### Recipe 4. Two programs hunt a number (Lab 3, 2025 homework)

> „Првата програма … ја стартува втората програма … Двата процеси (програми) креираат по 5 нитки кои ја пребаруваат вредноста „К“ во низа од „N“ цели броеви. Низата е запишана во датотека со име „dat.txt“ … Доколку е, тогаш го убива другиот процес и печати дека тој (прв или втор) процес го нашол елементот и печати на која позиција го нашол елементот.“

(Cyrillic extracted from the Lab 3 PDF; the rest of that page is blank.)

1. Program 1: check `argc == 3`, `N = atoi(argv[1])`, `K = atoi(argv[2])`, read `N` ints from `dat.txt` into a `malloc`'d array (the main thread does this).
2. `fork`; child: `execl("./lab31", "lab31", argv[1], argv[2], (char *)NULL); perror; _exit(127);`. Parent keeps the child's pid in a global (`PIDvtora`).
3. 5 threads (index array): each does `N` times: random index, compare with `K`. Found: `kill(other, SIGKILL)`, print the process number **and the position `guess`**, `exit(0)` (ends the whole process). Not found after `N` tries: return.
4. Main joins all 5; none found → `kill(other, SIGKILL)`, print "finished without finding it".
5. Program 2 (`lab31.c`) is the same, but the other pid is `getppid()`.

Course files: `course/2025/Labs/Lab3/threads/Lab3.c` and `lab31.c`. Kind remarks: the text asks to print the **position** too and the files print only the thread number (add `guess` to the message). Both files seed `rand()` with `srand(time(NULL))` once and the five threads share `rand()`'s hidden state: fine here, but use `rand_r(&seed)` with a private seed if you want it clean. Compile both: `gcc Lab3.c -pthread -o Lab3; gcc lab31.c -pthread -o lab31`, run `./Lab3 1000 7` in the directory that holds `dat.txt` and `lab31`.

### Recipe 5. Many threads reading one buffer (Lab 5 threads, Z1)

Lab text not in the repo; inferred from `course/2025/Labs/Lab5/threads/Z1.c`: the program takes a file name, reads the number of threads from the keyboard, maps the file (at most 1000 bytes) with `mmap`, and the threads take turns printing a random 10 to 99 byte chunk each, sleeping 2 s between turns, until the file is done.

1. Shared state: `globalOffset`, `realSize`, `finished`, all guarded by **one** mutex.
2. Thread loop: `lock`; if `finished` or `globalOffset >= realSize` → set `finished`, `unlock`, `break`; pick `X`, `toRead = min(remaining, X)`; print the chunk; `globalOffset += toRead`; `unlock`; `sleep(2)` **outside** the lock.
3. Main: validate `numThreads` (the file has `scanf("%d")` without a check and a VLA `pthread_t threads[numThreads]`: check `1 <= n <= 100` first), create, join, `munmap`, `pthread_mutex_destroy`.

Why it is correct: reading `globalOffset`, deciding and advancing it happen under one lock. Why `sleep` is outside: otherwise the other threads would wait idle during the sleep.

### Recipe 6. Wait with threads instead of a loop (Lab 2 2026, Bonus 2)

> „БОНУС2: дали чекањето на дете процесите може да се направи со нитки, по една нитка за секое дете, наместо со една јамка во родителот? Што се добива, а што се губи со тој пристап?“

Answer in three points (the file `course/2026/Labs/Lab2/Z2_threads.c` has the full version in its header):

1. Threads **can** do the waiting: `waitpid` works from any thread of the parent.
2. Threads **cannot replace** processes: `exec` replaces the whole process, so the children still must be processes, and must be forked **before** the threads.
3. Gain: blocking wait, no 100 ms polling loop, no list of "who is still running". Loss: shared state (done flags, results) needs a mutex, the fork-before-threads rule, and more code for the same result.

Details of the safe-`kill` design: section 7.4.

---

## 10. Pitfalls

| # | Symptom | Cause | Fix |
|---|---|---|---|
| 1 | All threads print the same index (often the last) | `&i` of the loop variable passed; they read it later (section 2) | pass `&idx[i]` of an array |
| 2 | Output missing, program ends early | `main` returned without `join`: the process ended and killed the threads (section 3) | join every thread (or `pthread_exit` in main) |
| 3 | Garbage or crash after join | thread returned `&local`; the thread's stack is gone | return `malloc`'d memory, or write into a global slot |
| 4 | Memory leak after joins | the `malloc`'d result was never freed | `free(ret)` in the joiner |
| 5 | Words split wrongly or counts differ run to run | `strtok` keeps hidden global state shared by all threads | `strtok_r(str, " ", &save)` with a private `save` (`15_strtok_r.c`: 6 and 2 words, correct) |
| 6 | Lines from different threads mixed inside one line | `printf` of several pieces from several threads interleave (each call is atomic only on its own) | build one string and print it with a single `printf`, or lock around the print |
| 7 | Works with `sleep` added, breaks otherwise | `sleep` was used to "synchronise" (hoping thread A is done) | `pthread_join`, or a mutex; sleep guesses, join knows |
| 8 | `undefined reference to pthread_create` | missing `-pthread` (section 6) | `gcc ... -pthread` |
| 9 | Counter or total has a different wrong value each run | data race on a shared variable (section 5) | one mutex around every read-modify-write of it; or per-thread slots and one sum in main |
| 10 | Thread reads a struct that "changed" or crashed | pointer to a struct in a function that already returned, or to a loop variable declared inside the loop | the struct array must live in `main` (or on the heap) until after `join` |
| 11 | Program hangs forever | forgotten `pthread_mutex_unlock` on an early `return`/`break` path, or deadlock (section 5) | unlock on **every** path; take several locks in a fixed order |
| 12 | `exec` in a thread, rest of the program never runs | exec replaces the whole process (section 8) | fork first, exec in the child |
| 13 | Child process has locked mutexes / hangs after `fork` | `fork` was called while other threads existed | fork before creating threads |
| 14 | Warning "control reaches end of non-void function" | thread function without `return` (as in `Kpromeni.c` and `1000br.c`) | end with `return NULL;` or `pthread_exit(NULL)` |
| 15 | Division by zero / `nan` in percentages | total is 0 when no thread found anything | `total ? x / total : 0` |
| 16 | Crash with `pthread_t tid[N]` | `N` from `scanf`/file unchecked (0, negative, huge) | validate `N` before the array |
| 17 | `pthread_create` returns non-zero but `errno` is 0 | pthread calls return the error number, they do not set `errno` | `rc = pthread_create(...)`; print `strerror(rc)` |

---

## 11. Quick reference

| Task | Call | Note |
|---|---|---|
| compile | `gcc -Wall -Wextra -pthread f.c -o f` | `-pthread`, not just `-lpthread` |
| start a thread | `pthread_create(&tid[i], NULL, f, &idx[i])` | returns `0` or an error number |
| thread function | `void *f(void *arg)` | end with `return value;` or `pthread_exit(value)` |
| argument | `int i = *(int *)arg;` | own object per thread, alive until read |
| wait for a thread | `pthread_join(tid[i], &ret)` | `ret` may be `NULL` |
| result | `malloc` and `return`, or slot `res[i]` | free after join |
| my thread id | `pthread_self()` | compare with `pthread_equal` |
| make a mutex | `pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;` or `pthread_mutex_init(&m, NULL)` | |
| lock / unlock | `pthread_mutex_lock(&m)` / `pthread_mutex_unlock(&m)` | small critical section |
| free a mutex | `pthread_mutex_destroy(&m)` | when no thread uses it |
| detach | `pthread_detach(tid)` | then never join |
| split an array in 10 | thread `i` takes `[i*100, i*100+100)` | each writes its own slot |
| strings in threads | `strtok_r` | not `strtok` |
| processes + threads | fork first, threads in the child | exec replaces everything |
| main ends | `join` all, or `pthread_exit(NULL)` | `return` kills the threads |
