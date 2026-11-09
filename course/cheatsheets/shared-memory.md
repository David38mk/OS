# Shared Memory Cheat Sheet – study guide for the OS labs and exams

Everything here was compiled with `gcc -Wall -Wextra -pthread` and run on Linux (Ubuntu 24.04, glibc 2.39). Each code block is a trimmed copy of a full file in `examples/shared-memory/`; the path is given under it. Output lines are real output. Pipes are in `pipes.md`.

---

## 0. Why shared memory

`fork()` gives the child a **copy** of the parent's memory (a private copy, made lazily). Whatever the child changes afterwards, the parent never sees:

```
$ ./01_fork_copy
child : counter=110 local=101
parent: counter=10 local=1   (unchanged)
```

(`examples/shared-memory/01_fork_copy.c`.) To let processes see **the same bytes**, the kernel can map one block of memory into the address space of several processes. The course shows two ways:

| Way | Name of the block | Typical calls |
|---|---|---|
| **System V shared memory** | a numeric **key** | `shmget`, `shmat`, `shmdt`, `shmctl` |
| **Memory mapping** (`mmap`) | a **file name**, or nothing at all (anonymous, parent and children only) | `open`, `ftruncate`, `mmap`, `munmap` |

```
  process A address space           kernel                 process B address space
  +---------------------+                                  +---------------------+
  | code | heap | stack |                                  | code | heap | stack |
  |  ... [ mapping ] ---+------>  [ one block of RAM ] <---+--- [ mapping ] ...   |
  +---------------------+          (key, or file)          +---------------------+
   writes here are visible in B, with no read()/write() call
```

Vocabulary: **attach** = map the block into your process; **detach** = unmap it; the block itself lives in the kernel until it is removed.

---

## 1. System V shared memory

| Function | Signature | Args | Returns | Errors (`errno`) |
|---|---|---|---|---|
| `ftok` | `key_t ftok(const char *path, int id)` | an existing path, one character | key / -1 | `ENOENT` (path missing) |
| `shmget` | `int shmget(key_t key, size_t size, int flags)` | key or `IPC_PRIVATE`; bytes; `IPC_CREAT \| 0666` (create if missing, permissions) | segment **id** / -1 | `EINVAL` (size bigger than the existing segment), `ENOENT` (no such key and no `IPC_CREAT`), `EEXIST` (`IPC_CREAT\|IPC_EXCL` and it exists), `EACCES`, `ENOMEM` |
| `shmat` | `void *shmat(int id, const void *addr, int flags)` | `NULL` = kernel picks the address; flags 0 (or `SHM_RDONLY`) | address / **`(void *)-1`** | `EACCES`, `EINVAL`, `EIDRM`, `ENOMEM` |
| `shmdt` | `int shmdt(const void *addr)` | the address `shmat` returned | 0 / -1 | `EINVAL` |
| `shmctl` | `int shmctl(int id, int cmd, struct shmid_ds *buf)` | `IPC_RMID` removes; `buf` = `NULL` | 0 / -1 | `EINVAL`, `EPERM`, `EIDRM` |

Headers: `<sys/ipc.h>`, `<sys/shm.h>` (`<sys/wait.h>` for `wait`).

- **Key.** Unrelated processes (Lab 3 and 4 run `./a.out 1 key`, `./a.out 2 key`, ... in separate terminals) agree on a number. The course files use `atoi(argv[2])` as key; `ftok(".", 'A')` computes one from a path, the same in every process that uses the same path and character.
- **`IPC_PRIVATE`** as key always creates a new segment with no public name. Only processes that inherit the **id** (a variable copied by `fork`) can use it: the parent-child case.
- **Pointer from `shmat`** is `void *`; assign it to your type (`struct shared *s = shmat(...)`), and compare with `(void *)-1` to detect failure. It is **not** `NULL` on failure.
- **Memory content**: a new segment is zero-filled. Fixed-size data only: `int`, arrays, structs without pointers (pitfall 7).
- **Attach before `fork()`** and the children inherit the mapping; or attach in each process after the fork. Both work.
- **A process that exits** is detached automatically. The segment is not deleted.

### Minimal pattern: child writes a struct, parent waits then reads

```c
struct shared { int count; int values[5]; char text[32]; };

int id = shmget(IPC_PRIVATE, sizeof(struct shared), IPC_CREAT | 0666);
if (id == -1) { perror("shmget"); return 1; }
if (fork() == 0) {
    struct shared *s = shmat(id, NULL, 0);
    if (s == (void *)-1) { perror("child shmat"); return 1; }
    s->count = 5;
    for (int i = 0; i < 5; i++) s->values[i] = (i + 1) * 11;
    strcpy(s->text, "napisano od deteto");
    shmdt(s);
    return 0;
}
wait(NULL);                            // child is done: now reading is safe
struct shared *s = shmat(id, NULL, 0);
printf("parent reads: count=%d text=\"%s\"\n", s->count, s->text);
shmdt(s);
shmctl(id, IPC_RMID, NULL);            // remove it
```

Full file: `examples/shared-memory/02_sysv_parent_child.c`

```
parent reads: count=5 text="napisano od deteto" values: 11 22 33 44 55
```

### Who removes the segment, and when

`shmctl(id, IPC_RMID, NULL)` marks the segment for removal: its key stops working at once and the memory is freed when the **last** process detaches. Exactly **one** process should call it, and only when nobody needs to open it by key again:

| Layout | Who calls `IPC_RMID` |
|---|---|
| Parent creates with `IPC_PRIVATE`, children inherit | parent, after `wait` |
| Several unrelated programs share a key (Lab 3 Z1) | the last user, or you from the shell with `ipcrm` |

If nobody does, the segment **stays in the kernel after every process is gone**.

### Seeing and removing segments from the shell

`examples/shared-memory/03_sysv_ftok_leak.c leak` creates a segment with `ftok` and exits without `IPC_RMID`:

```
$ ./03_sysv_ftok_leak leak
key = 0x41439e72
wrote "zdravo" into segment 2
leaking on purpose: run  ipcs -m  then  ipcrm -m 2
$ ipcs -m
------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status
0x41439e72 2          david      666        4096       0
$ ipcrm -m 2        # remove by id;  ipcrm -M 0x41439e72 removes by key
```

| Command | Does |
|---|---|
| `ipcs -m` | list segments (`nattch` = processes attached now) |
| `ipcrm -m ID` / `ipcrm -M KEY` | remove by id / by key |
| `ipcs -m -i ID` | details of one segment |

Without the argument the same program ends with `IPC_RMID` and leaves nothing behind.

---

## 2. Memory mapping (`mmap`)

`mmap` maps a **file** (or plain anonymous memory) into your address space. The file is the name; any process that opens the same file and maps it with `MAP_SHARED` sees the same bytes.

| Function | Signature | Args | Returns | Errors (`errno`) |
|---|---|---|---|---|
| `open` | `int open(const char *path, int flags, mode_t mode)` | `O_RDWR \| O_CREAT`, mode `0666` | fd / -1 | `ENOENT`, `EACCES` |
| `ftruncate` | `int ftruncate(int fd, off_t len)` | set the file length (grows it with zeros) | 0 / -1 | `EBADF`, `EINVAL` |
| `mmap` | `void *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t off)` | `NULL`; bytes; `PROT_READ \| PROT_WRITE`; `MAP_SHARED`; fd; `0` | address / **`MAP_FAILED`** | `EACCES`, `EBADF`, `EINVAL` (`len` 0, bad flags, `off` not a multiple of 4096), `ENOMEM`, `ENODEV` |
| `msync` | `int msync(void *addr, size_t len, int flags)` | `MS_SYNC` = write to the file now | 0 / -1 | `EINVAL`, `ENOMEM` |
| `munmap` | `int munmap(void *addr, size_t len)` | what `mmap` returned, same `len` | 0 / -1 | `EINVAL` |
| `close` | `int close(int fd)` | | 0 / -1 | |

Headers: `<fcntl.h>` (`open`, `O_*`), `<sys/mman.h>` (`mmap`, `PROT_*`, `MAP_*`), `<unistd.h>` (`ftruncate`, `close`). The check is against `MAP_FAILED`, not `NULL`.

### Minimal pattern: file-backed mapping

```c
int fd = open("/tmp/06_map.dat", O_RDWR | O_CREAT | O_TRUNC, 0644);
if (fd == -1) { perror("open"); return 1; }
if (ftruncate(fd, SIZE) == -1) { perror("ftruncate"); return 1; }   // file must be big enough

char *mem = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
if (mem == MAP_FAILED) { perror("mmap"); return 1; }

strcpy(mem, "tekst napisan so obicna dodelba");   // plain memory write, no write() call
mem[0] = 'T';
msync(mem, SIZE, MS_SYNC);                        // force it to the file now (optional)
munmap(mem, SIZE);
close(fd);
```

Full file: `examples/shared-memory/06_mmap_file.c`; it then reads the file with `read()` and prints `file now starts with: "Tekst napisan so obicna dodelba"`.

Notes: after `mmap` the fd may be closed, the mapping stays. The data lives in the **file**, so it survives the end of all processes (unlike an anonymous block). `msync` is optional: the kernel writes the pages back by itself, `msync` only forces the moment.

### Anonymous shared memory (parent and children only)

No file, no key: add `MAP_ANONYMOUS`, pass fd `-1`. Map **before** `fork()` so the children inherit the mapping.

```c
int *res = mmap(NULL, N * sizeof(int), PROT_READ | PROT_WRITE,
                MAP_SHARED | MAP_ANONYMOUS, -1, 0);
for (int i = 0; i < N; i++)
    if (fork() == 0) { res[i] = (i + 1) * (i + 1); return 0; }   // child i fills slot i
while (wait(NULL) > 0) {}
for (int i = 0; i < N; i++) printf("slot %d = %d\n", i, res[i]);
```

Full file: `examples/shared-memory/07_mmap_anon_children.c` (prints slots `1 4 9 16`).

### A whole file as a char array: `mmap` instead of `read`

With `read` you allocate a buffer, loop, and handle partial reads. With `mmap` the file **is** the array: map it, then use `txt[i]`.

```c
struct stat st;  fstat(fd, &st);                       // st.st_size = file length
if (st.st_size == 0) return 0;                         // mmap length 0 fails with EINVAL
char *txt = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
for (off_t i = 0; i < st.st_size; i++) if (txt[i] == '\n') lines++;
munmap(txt, st.st_size);
```

Full file: `examples/shared-memory/08_mmap_read_file.c` (`31 bytes, 3 lines, 3 spaces`). `PROT_READ` and `MAP_PRIVATE` are enough for read-only use. The text is **not** null-terminated: loop to `st_size`, do not `strlen` it or use `%s`.

---

## 3. System V vs `mmap` vs pipe

| | System V shm | `mmap` file | `mmap` anonymous | Pipe |
|---|---|---|---|---|
| Name | integer key (or `IPC_PRIVATE`) | file name | none | none (fds inherited) |
| Who can attach | any process that knows the key | any process that opens the file | parent and children (inherit) | parent and children (inherit) |
| Size | set in `shmget`, fixed | set by `ftruncate`, fixed (file can be longer) | set in `mmap`, fixed | 64 KiB buffer, stream |
| Lifetime | until `IPC_RMID` (**survives the processes**) | file stays on disk, content too | gone with the last process | gone with the last process |
| Cleanup | `IPC_RMID` / `ipcrm` | `rm` the file | automatic | automatic |
| Access | memory, any order | memory, any order | memory, any order | `read`/`write`, in order, consumed |
| Synchronisation | none | none | none | **built in** (blocking, EOF) |

Beyond this course: `shm_open` (POSIX shared memory with a name like `/myseg`, it needs `-lrt` on old systems).

---

## 4. Synchronisation: what the course does, and what goes wrong without it

The course shows **no locks** for shared memory. Two patterns are used:

1. **Sequencing with `wait`.** The parent starts the children, calls `wait(NULL)` (or `waitpid`) for all of them, and only then reads. Writers have finished, so there is no overlap by construction (`02`, `05`, `07` examples).
2. **A flag byte, polled with `sleep`** (Lab 3, Lab 4). Byte 0 of the block says "message present": the writer fills the text first, then sets `[0] = 1`; the reader sleeps in a loop until it sees `1`. This works for **one writer and one reader** exchanging one message. Its limits: the "deleter" process zeroes the block at any moment, even while someone is mid-write; the reader can see the flag before the text is final on other CPUs (no memory barrier). Fine for the lab, not a general design.

### What happens when two writers overlap

`examples/shared-memory/11_race_counter.c`: 4 children each add 1 to **one** shared counter 1 000 000 times, with no lock.

```c
volatile long *counter = mmap(NULL, sizeof(long), PROT_READ | PROT_WRITE,
                              MAP_SHARED | MAP_ANONYMOUS, -1, 0);
...
for (int k = 0; k < INCS; k++)
    (*counter)++;              // read, add, write back: three steps, not one
```

```
$ ./11_race_counter
expected 4000000, got 2143139
$ ./11_race_counter
expected 4000000, got 1667944
```

The result changes each run. Two children read the same old value, both add 1, both write: one update is lost. Rule: **each shared location has one writer at a time**. Give each child its own slot (`res[i]`) and the problem does not exist.

Beyond this course: process-shared mutexes (`pthread_mutexattr_setpshared`) and POSIX semaphores (`sem_open`, `sem_wait`, `sem_post`) are the real tools for several writers.

---

## 5. Exam recipes

**R1. Lab 3 Z1: three process types over System V shm** (`2025/Labs/Lab3/shm/Z1.c`). `./a.out TYPE KEY` : same key, three terminals. All attach a 1024-byte segment with `shmget(key, 1024, 0666 | IPC_CREAT)` and `shmat`. Byte 0 is the flag, bytes 1.. the message.

| Type | Does |
|---|---|
| 1 writer | if `shared[0] == 0`: prompt, `fgets(shared + 1, ...)`, `shared[0] = 1`, sleep `rand() % 10 + 1` s; else sleep 1 s |
| 2 deleter | every 5 s `memset(shared, 0, 1024)` (clears flag and text) |
| 3 reader | if `shared[0] == 1`: print `shared + 1`, sleep random; else sleep 1 s |

Teaches: `shmget`/`shmat` with a key from `argv`, flag-byte handshake, `srand(time(NULL) ^ getpid())` so processes get different random numbers. It compiles with no warnings. What it does not do: it never calls `IPC_RMID`, so after you stop the three programs the segment stays. Checked on the box:

```
$ ipcs -m
key        shmid      owner      perms      bytes      nattch     status
0x00001092 6          david      666        1024       0
$ ipcrm -M 4242      # key 4242 = 0x1092
```

**R2. Lab 3 Z2: the same with memory mapping** (`2025/Labs/Lab3/shm/Z2.c`). Second argument is a file name instead of a key: `open(name, O_RDWR | O_CREAT, 0666)`, `ftruncate(fd, 1024)`, `mmap(... MAP_SHARED ...)`, same three roles on `mem[0]`/`mem + 1`, ends with `munmap` + `close`. Teaches: file as the name of the shared block; checks `open`, `ftruncate` and `mmap` results. The block's content also stays in the file after the programs exit.

**R3. Lab 4 Z1: Lab 3 Z1 written again with System V** (`2025/Labs/Lab4/Z1.c`). The reader clears the flag itself (`dmem[0] = 0` after printing), so each message is printed once. **As shipped it does not compile or run correctly**, three separate mistakes:
- line 29, `printf("...")` has no `;`: `error: expected ';' before 'return'`.
- line 33, `srand(time[NULL])` should be `time(NULL)`: `error: array subscript is not an integer`.
- line 21, `if (memID)` is inverted. `shmget` returns **-1** on failure, so it must be `if (memID == -1)`. As written, a successful `shmget` (a positive id) prints `Memorijata ne e kreirana` and exits.

**R4. Lab 4 Z2: Lab 3 Z2 with `mmap`** (`2025/Labs/Lab4/Z2.c`; the same code is in `2025/Exercises/memory/MemMapDat.c`, which stops after the `mmap` call). It includes `<sys/ipc.h>` and `<sys/shm.h>` but needs `<fcntl.h>` and `<sys/mman.h>`:

```
error: 'O_RDWR' undeclared (first use in this function)
error: 'PROT_READ' undeclared (first use in this function)
error: 'MAP_SHARED' undeclared (first use in this function)
error: 'MAP_FAILED' undeclared (first use in this function)
```

Fix: add `#include <fcntl.h>` and `#include <sys/mman.h>`. The logic (ftruncate, mmap, three roles, munmap) is the same as R2.

**R5. "Parent needs the results of N children."** Allocate the block **before** the fork, give child `i` slot `i`, the parent `wait`s for all and reads the array. Same shape with either mechanism:

```c
int id = shmget(IPC_PRIVATE, N * sizeof(long), IPC_CREAT | 0600);
long *res = shmat(id, NULL, 0);                 // attach BEFORE fork: children inherit it
for (int i = 0; i < N; i++)
    if (fork() == 0) { res[i] = ...; shmdt(res); return 0; }   // own slot
while (wait(NULL) > 0) {}
for (int i = 0; i < N; i++) printf("child %d -> %ld\n", i, res[i]);
shmdt(res);  shmctl(id, IPC_RMID, NULL);
```

Full file: `examples/shared-memory/05_sysv_results_array.c` (prints `child 0 -> 5050` ... `child 3 -> 80200`). The `mmap` version is `07_mmap_anon_children.c`. Pipe version: `pipes.md` section 3.4.

**R6. "Parent builds arrays, child uses them"** (`2024/Exams/partial_one/Z3.c`). No sharing needed to get data **to** the child: arrays filled before `fork()` are copied into it. Shared memory or a pipe is only needed to get data **back** from the child, or to send data created after the fork. See `pipes.md` R5.

---

## 6. Pitfalls

1. **`MemMapSHM.c` does not compile.** `2025/Exercises/memory/MemMapSHM.c`:
   ```
   MemMapSHM.c:29:65: error: expected ';' before 'return'
   MemMapSHM.c:17:9: warning: unused variable 'tip' [-Wunused-variable]
   ```
   Line 29 is `printf("Pokazuvacot ne uspea da ja pristapi memorijata")` with **no semicolon**. The compiler reports the line **after** the missing `;`, because only the next token (`return`) shows that the statement went on. Fix: add `;` at the end of line 29. Rule: look at the line **above** the reported one first. The file then compiles (and does nothing: it stops after `shmat`).
2. **Segment never removed.** Symptom: `ipcs -m` lists a segment with `nattch 0` after your programs ended; creation with a bigger size then fails (pitfall 3). Cause: no `shmctl(id, IPC_RMID, NULL)`. Fix: the creator, or the last user, calls it; or `ipcrm -m ID` by hand. Do it every time while testing.
3. **`shmget` fails with `Invalid argument` (`EINVAL`).** A segment with that key already exists and is **smaller** than the size you ask for (for example left over from an earlier run with a different size). `examples/shared-memory/04_shmget_einval.c`:
   ```
   create 100 bytes: id=3
   shmget 5000 bytes: Invalid argument (errno=22)
   shmget 50 bytes : id=3 (same segment: yes)
   ```
   Asking for less is allowed; asking for more is not. Fix: remove the old segment (`ipcrm`), use the same size everywhere (one `#define`).
4. **`shmat` result not checked, or checked against `NULL`.** It returns `(void *)-1` on failure. `if (p == NULL)` never fires; `if (p == (void *)-1)` does (the course files write `(char *)-1`, which is the same value).
5. **`mmap` without `ftruncate`: `Bus error (core dumped)`, shell status 135.** `mmap` itself succeeds on an empty file; the first access to a page past the end of the file raises `SIGBUS`. `examples/shared-memory/09_sigbus_no_ftruncate.c`:
   ```
   mmap ok at 0x734aeba56000, writing now...
   Bus error (core dumped)
   ```
   Fix: `ftruncate(fd, len)` after `open`, before `mmap`. The same signal appears when the mapping is **longer than the file** and you touch the part beyond the last page of the file. A mapping **shorter** than the file is fine; you only see the first `len` bytes. The other `mmap` length error is `len == 0`: `EINVAL` (check `st_size` first).
6. **Forgot `MAP_SHARED`.** With `MAP_PRIVATE` the mapping is copy-on-write: writes go to a private copy, never to the file or other processes, and **no error is reported**. `examples/shared-memory/10_map_private.c`:
   ```
   MAP_SHARED  parent sees 99
   MAP_PRIVATE parent sees 1
   ```
   Use `MAP_SHARED` whenever the point is to share. `MAP_PRIVATE` is right only for read-only use such as `08_mmap_read_file.c`.
7. **Pointers inside shared memory.** Each process may attach the block at a **different address**; an address stored by one is wrong in the other. `examples/shared-memory/12_pointer_in_shm.c` attaches one segment twice, like two processes:
   ```
   attach #1 at 0x73fa636f0000, attach #2 at 0x73fa636ef000
   pointer 0x73fa636f0018 is inside mapping #2? NO
   index   1 -> b[1].value = 2  (always right)
   ```
   Fix: store **indices** (or offsets) and compute the address as `base + index` in each process. Also no `char *` into the heap, no `strdup` result: only the bytes themselves (`char text[32]` in the struct).
8. **Parent reads before the child wrote.** Symptom: zeros or old data. Fix: `wait(NULL)` (or `waitpid`) before reading, as in every example. Reading earlier is a race, not an error.
9. **Several processes write the same location.** Updates are lost (section 4, expected 4000000, got 1667944). Fix: one writer per location (slot per child), or a real lock (beyond this course).
10. **`ipcs`/`ipcrm` confusion.** `shmget` returns an **id**, the one `shmat`, `shmctl` and `ipcrm -m` take. The **key** is what you pass in to `shmget` and what `ipcrm -M` takes. Mixing them gives `EINVAL` or "invalid id".
11. **Missing headers hide behind warnings.** Lab 4 Z2 shows it: without `<sys/mman.h>` and `<fcntl.h>`, GCC 13 only warns `implicit declaration of function 'mmap'` and then the constants are plain errors. Always compile with `-Wall -Wextra`, read the **first** error.
12. **Text from `fgets` into shared memory.** `fgets(shared + 1, MEM_SIZE - 1, stdin)` keeps the `\n`; reader prints with `"%s"` (no extra newline). If stdin hits EOF `fgets` returns `NULL` and leaves old bytes, but the flag is still set. Check the result.

---

## 7. Quick reference

| Task | Code |
|---|---|
| System V block, parent and children | `id = shmget(IPC_PRIVATE, size, IPC_CREAT\|0666); p = shmat(id, NULL, 0); ... shmdt(p); shmctl(id, IPC_RMID, NULL);` |
| System V block, unrelated programs | same key everywhere: `key = atoi(argv[2])` or `ftok(".", 'A')`; **same size** everywhere |
| Check `shmat` | `if (p == (void *)-1) { perror("shmat"); ... }` |
| List / delete segments | `ipcs -m`; `ipcrm -m ID`; `ipcrm -M KEY` |
| File-backed block | `fd = open(f, O_RDWR\|O_CREAT, 0666); ftruncate(fd, N); p = mmap(NULL, N, PROT_READ\|PROT_WRITE, MAP_SHARED, fd, 0);` |
| Check `mmap` | `if (p == MAP_FAILED)` |
| Anonymous block (before `fork`) | `mmap(NULL, N, PROT_READ\|PROT_WRITE, MAP_SHARED\|MAP_ANONYMOUS, -1, 0)` |
| Read a file as `char[]` | `fstat` for size; `mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0)`; not null-terminated |
| Finish with `mmap` | `munmap(p, N); close(fd);` |
| Include for `mmap` | `<sys/mman.h>`, `<fcntl.h>`, `<unistd.h>` |
| Include for System V | `<sys/ipc.h>`, `<sys/shm.h>` |
| Results from N children | block before `fork`, child `i` writes `res[i]`, parent `wait`s then reads |
| Flag-byte handshake (Lab 3/4) | writer: text first, then `mem[0] = 1`; reader: `sleep(1)` until `mem[0] == 1` |
| Store inside the block | plain data and **indices**, never pointers |
| Beyond this course | `shm_open`, process-shared mutexes, POSIX semaphores |
