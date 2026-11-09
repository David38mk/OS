# Linux Character Drivers Cheat Sheet – study guide for the second partial

What a character driver is, the one module skeleton you need, the `read`/`write` handler contract, and the four buffer-bookkeeping patterns that the exam keeps asking for. Read sections 1 to 6 in order once, do section 8 with a clock, then use the rest as a lookup.

Every module in this sheet is a real file in `course/cheatsheets/examples/drivers/`. They **compile** against the kernel headers in the Linux box (`ssh oslab`, the docker container). They **cannot be loaded** there: the container shares the Windows (WSL) kernel, which has no module loading. Loading and testing happens on the lab PC (a real Linux). Each session below says where it runs.

---

## 1. What a character driver is

A **device file** is a name in `/dev` that is not a normal file. It has a **major number** (which driver owns it) and a **minor number** (which device of that driver). A **character device** is one you read and write as a stream of bytes, in order (a terminal, `/dev/null`, `/dev/random`). Your driver is a small piece of kernel code that decides what happens when a program touches that file.

```
 user space     $ cat /dev/primer1
                  open("/dev/primer1")          read(fd, buf, 131072)
 ─────────────────────── system call ──────────────────────────────
 kernel         VFS (virtual file system): path -> inode "char device, major 60, minor 0"
                  |  major 60 -> table filled by register_chrdev(60, ...)
                  v
                your struct file_operations primer_fops
                    .open  -> (not set: nothing special)
                    .read  -> primer_read(file, buf, count, offp)   <-- YOUR code
                    .write -> primer_write(file, buf, count, offp)  <-- YOUR code
                    .release -> (not set)
```

The VFS is the kernel layer that makes every file look the same. A program calls `read()`; the VFS finds the `file_operations` table registered for that major number and calls your function. You never call your handlers yourself.

**Kernel space is not user space.** The rules that change how you write C:

| In a user program | In the kernel |
|---|---|
| libc (`printf`, `malloc`, `strlen`...) | no libc. `printk` replaces `printf`; `kmalloc` replaces `malloc`; a few `str*` helpers exist in `<linux/string.h>` |
| `#include <stdio.h>` | does not exist: `fatal error: stdio.h: No such file or directory`. Use `<linux/...>` headers |
| `main()` | no `main`. A module has an init function (runs at `insmod`) and an exit function (runs at `rmmod`) |
| a pointer is just a pointer | a pointer from the user (`buf` in your handler) must **never** be dereferenced directly: it points into another process's memory. Copy with `copy_to_user` / `copy_from_user` |
| a bug kills your process | a bug can freeze or crash the whole machine. Test on the lab PC, not on something you care about |

Marker `__user` in a prototype (`char __user *buf`) only documents "this pointer comes from user space". `gcc` ignores it; a checker called `sparse` uses it.

---

## 2. Module skeleton

A **module** is a piece of kernel code loaded while the system runs. The smallest useful one:

```c
#include <linux/module.h>          /* module_init, MODULE_LICENSE ... */
#include <linux/fs.h>              /* file_operations, register_chrdev */
#include <linux/uaccess.h>         /* copy_to_user, copy_from_user */

static int __init primer_init(void)      { printk(KERN_INFO "hello\n"); return 0; }
static void __exit primer_exit(void)     { printk(KERN_INFO "bye\n"); }

module_init(primer_init);          /* run primer_init at insmod */
module_exit(primer_exit);          /* run primer_exit at rmmod */
MODULE_LICENSE("GPL");
```

| Piece | Meaning |
|---|---|
| `module_init(f)` / `module_exit(f)` | tell the kernel which functions to call on load / unload. Names are yours (`r_init` in the course files) |
| `__init` / `__exit` | hints: init code can be freed from memory after loading. Optional for the exam, expected in "proper" code |
| `static` | the function is private to this file. **Always** put it on handlers and init/exit, otherwise they pollute the kernel's global names (see pitfall 3) |
| init returns `int` | `0` = success. A negative value (`-EBUSY`, `-ENOMEM`) = load failed, the module is not kept |
| `MODULE_LICENSE("GPL")` | without it the kernel marks itself "tainted" and prints a warning; some functions are refused to non-GPL modules |
| `MODULE_AUTHOR`, `MODULE_DESCRIPTION` | metadata shown by `modinfo` |
| `printk(KERN_INFO "text\n")` | write to the kernel log. `KERN_INFO` is the importance level (`KERN_ERR`, `KERN_WARNING`, `KERN_DEBUG` also exist). Note: no comma after the level |

Read the log with `dmesg` (or `dmesg | tail`; some systems need `sudo`). It is the only "printf" you have.

**Build.** The kernel has its own build system (kbuild). You do not call `gcc` yourself. A two-line Makefile names the module and points at the kernel headers (`examples/drivers/Makefile`):

```make
KDIR ?= /lib/modules/$(shell uname -r)/build      # headers of the RUNNING kernel
obj-m += 01_linear.o                              # build 01_linear.c into 01_linear.ko

all:
	$(MAKE) -C $(KDIR) M=$(CURDIR) modules        # recipe lines start with a TAB
```

`make` produces `01_linear.ko` (kernel object). Tools:

| Command | Does | Needs root |
|---|---|---|
| `sudo insmod x.ko` | load the module (runs init) | yes |
| `lsmod \| grep x` | is it loaded? | no |
| `sudo rmmod x` | unload (runs exit). Name without `.ko` | yes |
| `modinfo x.ko` | show license, author, description | no |
| `dmesg \| tail` | see your `printk` lines | maybe |

---

## 3. Registering the device

The init function must tell the kernel "major number 60 belongs to my `file_operations`".

```c
static const struct file_operations primer_fops = {
    .owner   = THIS_MODULE,        /* pins the module while the file is open: rmmod refused */
    .open    = primer_open,        /* optional */
    .release = primer_release,     /* optional: runs on the last close() */
    .read    = primer_read,
    .write   = primer_write,
};
```

`.name = value` is a **designated initializer**: it fills only the named field, the rest stay `NULL` (= "not supported / default behaviour"). So `.read` without `.write` gives a read-only device: `write()` fails with `EINVAL`.

| Call | Meaning |
|---|---|
| `register_chrdev(60, "primer1", &primer_fops)` | claim major **60**. Returns `0` on success, negative on error (`-EBUSY` = number taken). `"primer1"` is the name shown in `/proc/devices` |
| `register_chrdev(0, "primer1", &primer_fops)` | **dynamic**: the kernel picks a free major and returns it (a positive number). Look it up with `grep primer1 /proc/devices` |
| `unregister_chrdev(60, "primer1")` | release it. **Must** be in the exit function |

The course uses the fixed major 60, which is in the range reserved for local and experimental use, so it does not clash with real hardware. In the exam fragments the call sits in the init function named `r_init`, and the table is called `myfops` (never defined, see section 8).

**Make the file in `/dev`.** Loading the module registers the major; it does not create the file. You do:

```
$ sudo mknod /dev/primer1 c 60 0      # c = character device, major 60, minor 0
$ sudo chmod 666 /dev/primer1         # everyone may read and write (else you need sudo for echo/cat too)
$ ls -l /dev/primer1
crw-rw-rw- 1 root root 60, 0 ... /dev/primer1
```

**Modern form, beyond this course.** Real drivers use `alloc_chrdev_region` (get a number range), `cdev_init`/`cdev_add` (attach the `file_operations`), and `class_create` + `device_create` (so `udev` makes the `/dev` file automatically; in kernel 6.8 `class_create` takes just the name). It is the same `file_operations` and the same handlers; only the registration is longer. `register_chrdev` is the old, short way and is what the exam uses.

---

## 4. The `read` handler contract

```c
static ssize_t primer_read(struct file *f, char __user *buf, size_t count, loff_t *offp);
```

| Parameter | Meaning |
|---|---|
| `f` | the open file (rarely needed here) |
| `buf` | **user** address to put the data into. Only `copy_to_user` may touch it |
| `count` | how many bytes the user program **asked for**. It is a maximum, not a promise |
| `offp` | pointer to the position of **this open file** (kept by the kernel per `open()`); section 6d |
| return | bytes **actually delivered** (`0..count`), or **`0` = end of file**, or a negative error (`-EFAULT`...) |

Three rules, in order of how many marks they cost:

1. **Return what you really copied.** If the user asked for 100 and you have 40, copy 40 and `return 40`.
2. **Return `0` when there is nothing more to give.** `cat` calls `read` in a loop and stops only when it gets `0`. A handler that never returns `0` makes `cat` print forever.
3. **Never copy more than `count`**, and never more than you have.

```
 mem[ ] 0         r                w                   512
        [ consumed |  unread data  |   free          ]
 read can deliver at most w - r bytes, and at most count.
 n = min(count, w - r);  n == 0  ->  return 0   (EOF)
```

**`copy_to_user(to_user, from_kernel, n)`** returns the number of bytes it could **not** copy. `0` means success. It fails (non-zero) if the user pointer is invalid. The proper reaction is `return -EFAULT;` ("bad address"). Ignoring the result compiles and works in the exam; in real code it is a bug.

**What the user commands turn into** (count in the `read` call):

| Command | `read()` calls your handler sees |
|---|---|
| `cat /dev/x` | `read(fd, buf, ~128 KiB)` again and again until one returns 0 |
| `head -c 5 /dev/x` | `read(fd, buf, 5)`, then stops (it has its 5 bytes) |
| `dd if=/dev/x bs=1 count=3` | three calls with `count = 1` |
| `dd if=/dev/x bs=512 count=1` | one call with `count = 512` |
| `./05_test_client` | calls with `count = 15` until 0 (example client) |

So your handler must work for `count = 1`, for `count` bigger than the data, and for `count` bigger than the whole buffer. The exact number `cat` uses depends on the version; never rely on it.

---

## 5. The `write` handler contract

```c
static ssize_t primer_write(struct file *f, const char __user *buf, size_t count, loff_t *offp);
```

Same shape, opposite direction. `count` is how many bytes the user **offers**; `buf` is the user's data (note `const`: you only read it).

| Return | Means |
|---|---|
| `n` where `0 < n <= count` | I accepted `n` bytes. If `n < count`, a well-behaved program calls `write` again with the remaining `count - n` bytes |
| `count` | all accepted |
| `-ENOSPC` (or another negative) | cannot accept, error to the user (`echo` prints "No space left on device") |
| `0` | "wrote nothing". Avoid it: some programs then retry forever (pitfall 9) |

**`copy_from_user(to_kernel, from_user, n)`** copies **n** bytes and, like `copy_to_user`, returns the number **not** copied (`0` = success).

**What `echo` really sends.** `echo abc > /dev/x` opens the file, calls `write(fd, "abc\n", 4)` once and closes. The **newline counts**: `count` is 4, not 3. Use `echo -n abc` or `printf abc` for exactly 3. Bookkeeping that is off by one often comes from forgetting this newline when you test by hand.

```
 write(fd, "hello\n", 6)  on a buffer with w = 510 of 512:
   free = 2  ->  accept 2 bytes, w = 512, return 2
   the program calls write(fd, "llo\n", 4) -> buffer full -> -ENOSPC
```

---

## 6. Buffer bookkeeping patterns

The exam gives a fixed array `char mem[N]` and a rule such as "writes go in, reads come out, when X happens do Y". Always the same tools: a **write index** `w` (next free slot), a **read index** `r` (next byte to deliver) and sometimes a **flag**. The handler logic is three lines:

```
n = min(count, bytes_available);      /* never more than asked, never more than there is */
copy n bytes;  move the index by n;
return n;                             /* what you moved, not count */
```

In `unsigned` arithmetic watch the types: `count` is `size_t` (unsigned). Compare it with `size_t` values, not with a negative-capable `int` (pitfall 5).

All four modules use `register_chrdev(60, "primer1", ...)`, so only one can be loaded at a time. `rmmod` before loading the next.

### 6a. Linear buffer – `01_linear.c`

**Rule:** a write appends at `w`. A read delivers from `r` up to `w`. When `r == w` there is nothing unread: return 0.

```
 start      r=w=0   [................................]
 echo hello  w=6    [hello\n.........................]     r=0
 cat         r=6    [hello\n.........................]     read #1 -> 6 bytes, read #2 -> 0 (cat stops)
 echo bye    w=10   [hello\nbye\n.....................]     r=6
 cat         r=10   prints only "bye": r and w live across open()s
```

Core of the handlers (the full file with `static`, `.owner` and error paths is `examples/drivers/01_linear.c`):

```c
size_t avail = w - r;
size_t n = count < avail ? count : avail;
if (n == 0) return 0;                       /* read: EOF */
if (copy_to_user(buf, mem + r, n)) return -EFAULT;
r += n;  return n;

size_t space = SIZE - w;                    /* write */
size_t n = count < space ? count : space;
if (space == 0) return -ENOSPC;
if (copy_from_user(mem + w, buf, n)) return -EFAULT;
w += n;  return n;
```

**Variant (2024 Z3): accept at most half of the free space per write.** `MAX = (1024 - w) / 2; n = min(count, MAX);`. Edge case to handle: with 1 byte free, `MAX` is 0, so you would accept nothing and `return 0` forever. Round up instead: `MAX = (free + 1) / 2`.

### 6b. Drain flag – `02_drain_flag.c` (this is `DRIVERS.c`)

**Rule:** while `flag == 0` the device only accepts writes, reads return 0. When the buffer becomes full, `flag` becomes 1. Now reads drain the data and writes are refused. When everything has been read, reset `r = w = flag = 0`.

```
 filling  flag=0   [hello....................]   w grows, read -> 0
 full     flag=1   [hellohellohellohello.....]   w == 512, write -> -ENOSPC
 draining flag=1   r grows toward w, read -> n bytes
 r == w   reset    r = w = flag = 0, back to filling
```

```c
/* read */  if (flag == 0) return 0;
            n = min(count, w - r);  copy;  r += n;
            if (r == w) r = w = flag = 0;           /* drained: reset */
            return n;
/* write */ if (flag == 1) return -ENOSPC;
            n = min(count, SIZE - w);  copy;  w += n;
            if (w == SIZE) flag = 1;                /* became full */
            return n;
```

The original `DRIVERS.c` reads **from the end backwards** (a stack): the last byte written comes out first. Sketch of that LIFO variant, which differs only in the read:

```c
/* after the buffer became full: r = 512 (the file has the comment "//r = 512" but sets r = 0 - a bug) */
n = count < r ? count : r;      /* r = bytes still inside */
r -= n;
copy_to_user(buf, mem + r, n);  /* the block just below r: newest data first */
if (r == 0) { w = 0; flag = 0; }
return n;
```

### 6c. Ring (circular) buffer – `03_ring.c`

**Rule:** the array is treated as a circle. `w` and `r` wrap to 0 when they reach the end. A counter `cnt` says how many bytes are stored: `cnt == 0` is empty, `cnt == SIZE` is full. (Comparing `r == w` alone cannot tell empty from full.)

```
 SIZE = 8       cnt = 5
 index  0 1 2 3 4 5 6 7
        [C D E . . . A B]        r = 6, w = 3
               ^w=3  ^r=6    A is the oldest byte
 reading 4 bytes: A B (to the end), then wrap: C D   -> two copies
 r = (r + n) % SIZE;   cnt -= n;
```

A transfer that crosses the end of the array is done as **two copies**: `first = SIZE - r` bytes to the end, then the rest from index 0. `first` is clamped to `n`. When there is no wrap the second copy has size 0 and does nothing.

```c
n = min(count, cnt);                 first = min(SIZE - r, n);
copy_to_user(buf,         ring + r, first);
copy_to_user(buf + first, ring,     n - first);
r = (r + n) % SIZE;  cnt -= n;       /* read */
/* write: n = min(count, SIZE - cnt), same two copies, w = (w + n) % SIZE; cnt += n; */
```

### 6d. Position in `*offp` – `04_offp_catsafe.c`

So far `r` and `w` are **globals**: one position shared by everyone, forever, until `rmmod`. The kernel also keeps a position **per open file**: every `open()` makes a new `struct file` whose position starts at 0, and it passes the address of that position as `offp`. Using it gives two properties for free:

- each `open()` starts reading at 0 (no global "r" to reset);
- `cat` terminates: it keeps asking, `*offp` advances, and at the end you return 0.

```
 echo hello > /dev/primer1      its own open file: *offp 0 -> 6      len = 6
 cat /dev/primer1               new open file:     *offp 0
     read #1  pos 0 < len 6 : deliver 6, *offp = 6
     read #2  pos 6 >= len  : return 0     EOF
 cat again                      new open file: *offp 0 again, prints hello again
```

```c
size_t pos = *offp;
if (pos >= len) return 0;                    /* read: EOF for this open file */
n = min(count, len - pos);  copy_to_user(buf, mem + pos, n);
*offp += n;  return n;                       /* you must update it yourself */
/* write: same with SIZE instead of len; afterwards if (*offp > len) len = *offp; */
```

The difference from 6a: 6a is a **queue** (reading consumes data, the next `cat` shows only new bytes); 6d is a **file** (reading does not consume, every `cat` shows the whole content, a new `echo` overwrites from 0).

**Variant: reset per open with `.open`** (no `offp`, but global indices start fresh for every program):

```c
static int primer_open(struct inode *i, struct file *f) { r = 0; return 0; }   /* + .open = primer_open */
```

**Variant: LIFO / stack.** `w` is the top. Write pushes: copy to `mem + w`, `w += n`. Read pops from the top: `n = min(count, w); w -= n; copy_to_user(buf, mem + w, n); return n;` (the block comes out in stored order; to reverse bytes too, copy them one by one from the top). Empty stack (`w == 0`) returns 0.

---

## 7. Build and test

**On the lab PC** (real Linux, you may be asked for the sudo password). Replace `01_linear` with the module you test:

```
$ cd course/cheatsheets/examples/drivers
$ make                                   # builds the four .ko files
$ sudo insmod 01_linear.ko
$ dmesg | tail -2                        # shows "primer1: loaded (linear)"
$ grep primer1 /proc/devices             # "60 primer1" = registered
$ sudo mknod /dev/primer1 c 60 0         # once per boot
$ sudo chmod 666 /dev/primer1
$ echo -n hello > /dev/primer1
$ echo " world" > /dev/primer1
$ cat /dev/primer1                       # hello world (+ newline)
$ cat /dev/primer1                       # empty: r == w, linear queue is drained
$ gcc -Wall -Wextra -o client 05_test_client.c && ./client abcdefghijklmnop
$ sudo rmmod 01_linear                   # runs exit; dmesg shows "unloaded"
$ make clean
```

`rmmod: ERROR: Module is in use` means some program still has `/dev/primer1` open (`.owner = THIS_MODULE` does that). `insmod: ERROR: ... Device or resource busy` means major 60 is already taken (the previous module is still loaded). `insmod: ... Operation not permitted`: forgot `sudo`.

The user-space client `05_test_client.c` writes a string, reopens the device and reads it back in 15-byte pieces until `read` returns 0. It gives the same output on every run with `04_offp_catsafe` (the new `open` starts at 0). With `01_linear` only the first run shows the text, because reading consumes it. With `02_drain_flag` it prints nothing until 512 bytes were written. It cannot run in the container (no `/dev/primer1`); it only compiles there.

**In the container** (`ssh oslab`) only compile is possible:

```
$ cd ~/OS/course/cheatsheets/examples/drivers
$ make KDIR=/lib/modules/$(ls /lib/modules | head -1)/build     # the box has one kernel-headers dir
$ gcc -Wall -Wextra -o /tmp/client 05_test_client.c              # client compiles, cannot run: no /dev/primer1
$ make KDIR=/lib/modules/$(ls /lib/modules | head -1)/build clean
```

`uname -r` inside the container reports the WSL kernel, which has no headers, hence the explicit `KDIR`. The build prints `warning: the compiler differs from the one used to build the kernel`; it is a note about the toolchain, not about your code. `insmod` in the container fails, that is expected.

**Reading the error messages** (the first two were produced in the box):

| Message | Cause and fix |
|---|---|
| `fatal error: stdio.h: No such file or directory` | user-space header in kernel code. Delete it; kernel code includes `<linux/...>` only |
| `error: implicit declaration of function 'printf'` | no libc in the kernel. Use `printk(KERN_INFO "...")` |
| `error: initialization of 'ssize_t (*)(struct file *, const char *, ...)' from incompatible pointer type` | the handler signature does not match `file_operations`. Check argument order and types (e.g. write needs `const char __user *`) |
| `warning: no previous prototype for 'read'` | handler is not `static`. Add `static` and give handlers distinct names (`primer_read`) |
| `implicit declaration of function 'copy_to_user'` | missing `#include <linux/uaccess.h>` (in this kernel `linux/fs.h` also pulls it in, but include it explicitly) |
| `Unknown symbol in module` at `insmod` (see `dmesg`) | you called something the kernel does not export, often a leftover libc function |
| `module license 'unspecified' taints kernel` in `dmesg` | missing `MODULE_LICENSE("GPL")` |

---

## 8. Exam recipes

**Convention in the exam files.** The files under `course/20xx/Exams/partial_two/` are **fragments**. They start with the same pile of user-space `#include`s (`stdio.h`, `unistd.h`, `sys/wait.h`) plus `linux/module.h`, the handlers are called plain `read` and `write`, `myfops` is used but never defined, and the file may end with `int main(){ return 0; }`. They do not compile. That is how the exam is graded: the professor reads the **handler logic and the buffer bookkeeping**, not the module boilerplate. So:

- Learn the full proper form (sections 2 to 3) so you can write it when you have time.
- Learn to write **just the handlers** fast (section 6). Add `static` and distinct names if you have a minute left; never lose time on headers.
- Always show the globals (`char mem[N]; int r, w, flag;`), the init that resets them, and `register_chrdev(60, "primer1", &fops)`.

Of the eleven C files under the `partial_two` folders only five are drivers; the others are pipes and shared memory (`pipes.md`, `shared-memory.md`). No task text is stored for these files; the tasks below are inferred from the code.

| File | Inferred task | Handler logic and what to watch |
|---|---|---|
| `2025/Exercises/drivers/DRIVERS.c` and `2020/.../Z3/Z3.c` (first `read`) | 512 B buffer. Write fills it; when full a flag flips and reads drain it **last-in-first-out**, then everything resets | 6b. `512 - (512 - r)` is just `r`. `r = 0` should be `r = 512` (the comment says so). Returns `x` after copying `x` (fine) but `0` on write when flagged |
| `2020/.../Z3/Z3.c` (second `read`) | an alternative attempt: drain from the end in blocks | two functions called `read`: cannot both exist. `flag == 1;` is a comparison, not an assignment (`=`). `retrun 0;` typo |
| `2020/.../Z3/Z3_3.c` | 4 KiB buffer; a write stores the message and remembers its length **rounded up to a multiple of 64** (`flag`); read returns that many bytes | write can fall off the end of a non-void function without `return`; read never returns 0 so `cat` loops; `r + flag <= 4096` mixes both. Fix: after the read `flag -= n`, return 0 when `flag == 0` |
| `2021/.../Z3.c` | `mem[6*1024]` plus a second `dop[4*1024]`; read delivers `w - r`; write left empty (likely: overflow goes to `dop`) | read: `w - r > count` should be `>=`. Types written without `struct` (`file *f`) and `...` as arguments. The `else` branch returns 0 at the end (good) |
| `2024/.../Z3.c` | linear buffer 1024 B, write accepts at most **half the free space**, read gives `r..w` | 6a. Real bugs: read `r + count < w` should be `<=`; in the else branch `r` is **not advanced**, so `cat` receives the same bytes forever (never 0). `int MAX` vs `size_t count`: use `size_t`. `MAX = 0` when 1 byte is free |
| `2024/.../Z4.c` | 512 B buffer with a **lock** `z` and a 6-letter password `shifra`: while locked, reads return the data **encrypted** (`shifiraj`) and a write is compared to the password to unlock; unlocked writes append and lock again | the string compare: `echo pass` sends `"pass\n"` so `strcmp` never matches; use `strncmp(pom, shifra, 6)` or copy into a `count + 1` buffer and add `\0`. `sifra` vs `shifra` typo. `r` never moves. A write that returns 0 (wrong password) can loop its writer: return `count` or `-EPERM` |
| `2020 Z2_2`, `Z3_1`, `Z4`; `2021 Z1`, `Z2`; `2022 Z2` | **not drivers**: shared memory + mmap'd file with a command protocol, XOR-style key cipher between two processes, two pipes with a counting child, N children running a command in turn, `cat \| tail \| wc`, N-command pipeline | see `shared-memory.md` and `pipes.md`; the driver headers are copy-paste habit |

### Handler skeleton (exam speed)

```c
char mem[512];  int r, w, flag;

static int r_init(void) { r = w = flag = 0; register_chrdev(60, "primer1", &myfops); return 0; }

static ssize_t primer_read(struct file *f, char *buff, size_t count, loff_t *offp) {
    size_t n = count < (size_t)(w - r) ? count : (size_t)(w - r);
    if (n == 0) return 0;
    copy_to_user(buff, mem + r, n);  r += n;  return n;
}
static ssize_t primer_write(struct file *f, const char *buff, size_t count, loff_t *offp) {
    size_t n = count < (size_t)(512 - w) ? count : (size_t)(512 - w);
    copy_from_user(mem + w, buff, n);  w += n;  return n;
}
```

### The 20-minute checklist

1. Read the task twice. Underline every **event** ("when full", "after the password", "half of the free space").
2. List the state you need: buffer(s), `r`, `w`, `flag`, key. One line each, with the type.
3. Write the globals and `r_init` (reset everything, `register_chrdev(60, "primer1", &fops)`).
4. Draw the buffer line with `r` and `w` and the **empty**, **full** and **middle** states.
5. Write `read`: `n = min(count, available)`; `if (n == 0) return 0`; copy; move `r`; `return n`.
6. Write `write`: `n = min(count, free)`; copy; move `w`; `return n`.
7. Add the task's special rule (flag flip, reset, wrap, lock) at the exact line where it happens.
8. Trace by hand: `echo hello`, `cat`, `cat` again. Does the second `cat` end with 0?
9. Trace the edges: `count = 1`, `count` bigger than the data, a write that does not fit.
10. Spend leftover time on `static`, `-EFAULT`, `.owner`, `module_exit` with `unregister_chrdev`.

---

## 9. Pitfalls

1. **Naming the handlers `read` and `write`.** They clash with the libc names in the exam's mixed headers and, if not `static`, with kernel names. Use `primer_read`. The course fragments do it; it is fine on paper, not in a real build.
2. **User-space headers in kernel code.** `<stdio.h>`, `<unistd.h>`, `<stdlib.h>` do not exist there. Only `<linux/...>` headers.
3. **Forgetting `static`** on handlers and init/exit. The kernel build warns (`no previous prototype`); global names can collide with the rest of the kernel.
4. **Returning `count` after copying fewer bytes.** The user program believes all of it was transferred. Return the number you moved.
5. **`int` for sizes.** `count` is `size_t` (unsigned), the position is `loff_t` (signed 64-bit). `w - r` stored in an `int` and compared with `count` converts to unsigned: a negative becomes huge. Use `size_t` locals, or cast both sides.
6. **Never returning 0.** `cat` loops forever. Every read must reach a state where it returns 0 (usually `r == w`). 2024 Z3's else branch is the classic: it returns `w - r` again without moving `r`.
7. **Ignoring the `copy_to_user` / `copy_from_user` result.** It returns the bytes **not** copied, `0` = ok. Proper: `if (copy_to_user(...)) return -EFAULT;`.
8. **Global indices never reset.** After the buffer was used once, the next test starts mid-array. Reset in init and wherever the rule says (drained, per `open`).
9. **Returning 0 from `write`** when you cannot accept data. A program that retries "the rest" may spin forever. Return `-ENOSPC` (or a count `> 0`). Typical write loops do keep retrying a short count; the effect of a plain `0` depends on the program, so avoid it. (Not tested in the container: no device.)
10. **Off-by-one at the end of the buffer.** `w <= SIZE` is legal, `mem[w]` with `w == SIZE` is out of bounds. Check `SIZE - w` before copying, and remember the newline that `echo` adds.
11. **`printk` flooding.** One line per `read` is fine; one line per byte or in a loop floods `dmesg`. End every `printk` with `\n`.
12. **Missing `MODULE_LICENSE`.** Taints the kernel and prints a warning.
13. **Forgetting `unregister_chrdev` in the exit function.** After `rmmod`, major 60 stays claimed by code that no longer exists; the next `insmod` fails with `Device or resource busy` and only a reboot frees it. Also `register_chrdev` can fail: return its error from init.
14. **Races.** Two programs reading at once can corrupt `r`/`w`. Real drivers use a `mutex`; the exam ignores it.

---

## 10. Quick reference

| Item | Form |
|---|---|
| read prototype | `static ssize_t primer_read(struct file *f, char __user *buf, size_t count, loff_t *offp)` |
| write prototype | `static ssize_t primer_write(struct file *f, const char __user *buf, size_t count, loff_t *offp)` |
| copy to user | `copy_to_user(user_dst, kernel_src, n)` returns bytes **not** copied (0 = ok) |
| copy from user | `copy_from_user(kernel_dst, user_src, n)` returns bytes **not** copied (0 = ok) |
| read returns | bytes delivered; `0` = EOF; `-EFAULT` bad user pointer |
| write returns | bytes accepted; `-ENOSPC` full; `-EFAULT` bad pointer |
| register | `register_chrdev(60, "primer1", &fops)` (major 0 = dynamic); undo: `unregister_chrdev(60, "primer1")` |
| file_operations | `.owner = THIS_MODULE, .open, .release, .read, .write` |
| module glue | `module_init(f); module_exit(g); MODULE_LICENSE("GPL");` |
| log | `printk(KERN_INFO "...\n")`, read with `dmesg` |
| build | `obj-m += x.o`, `make -C $(KDIR) M=$(CURDIR) modules` |
| load / unload | `sudo insmod x.ko`, `lsmod`, `sudo rmmod x`, `modinfo x.ko` |
| device file | `sudo mknod /dev/primer1 c 60 0; sudo chmod 666 /dev/primer1` |
| cat / echo | `cat` reads until 0; `echo abc` writes 4 bytes (newline counts) |
| `n` formula | `n = min(count, available)`; `return n`; `0` when nothing left |

| Pattern | Indices | End condition | Example |
|---|---|---|---|
| linear queue | `r`, `w` | `r == w` empty, `w == SIZE` full | `01_linear.c` |
| drain flag | `r`, `w`, `flag` | full -> `flag = 1`; `r == w` -> reset | `02_drain_flag.c`, `DRIVERS.c` |
| ring | `r`, `w`, `cnt` | `cnt == 0` empty, `cnt == SIZE` full, indices `% SIZE` | `03_ring.c` |
| per-open position | `*offp`, `len` | `*offp >= len` -> 0 | `04_offp_catsafe.c` |
| stack (LIFO) | `w` as top | `w == 0` empty | sketch in 6d |
