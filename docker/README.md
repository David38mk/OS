# The Linux test box: how to use it and how it works

A Linux machine for running the course scripts and C programs, built from `docker/Dockerfile` and `docker-compose.yml` in the repo root. Part 1 is the daily routine, part 2 is reference, part 3 explains what is actually happening under the hood.

---

## 1. Daily routine

**Start.** Docker Desktop must be running (the whale icon in the tray). The container `oslab` starts automatically with it. If it is not up, run this once from the repo folder in any Windows terminal:

```powershell
docker compose up -d
```

**Connect.** Open a terminal (PowerShell, Windows Terminal or Git Bash) and:

```bash
ssh -p 2222 david@localhost
```

Your key `~/.ssh/id_ed25519` is authorised, so there is no password. Fallback password is `os`. You land in `/home/david/OS`, which is this repo.

**Edit on Windows, run on Linux.** Keep editing the files in VS Code on Windows as you always did. The Linux shell sees every save instantly, because it is the same folder (see part 3), not a copy.

**Run things** in the Linux shell:

```bash
# shell scripts
bash course/2026/Labs/Lab1/Z1.sh a.txt b.txt
cd course/2026/Labs/Lab1 && bash tests/run_tests.sh

# python
python3 course/2024/Exams/partial_one/Z2.py3 all.crypto

# C, via the helper: compiles with -Wall -pthread into <folder>/bin/ and runs the result
./run.sh course/2024/Exams/partial_one/Z3.c btc.txt eth.txt

# C, by hand, from the task folder
gcc -Wall -pthread Z3.c -o Z3 && ./Z3 btc.txt eth.txt
```

**Tasks with several processes or programs.** Open a second terminal and `ssh` again. Each session is an independent shell on the same machine, so one window can run the writer and another the reader. For the Lab 3 pair, compile both in the same folder (`gcc -pthread lab31.c -o lab31` first, since program 1 execs `./lab31`).

**Test data.** Create the input files a task expects (`naredbi.txt`, `*.crypto`, `*.ip`, ...) in the task folder or in a scratch folder such as `~/tmp`. Compiled binaries and `bin/` folders are ignored by git, data files you create are not, so delete them or keep them out of the repo.

**Stop.** Nothing to do; closing the terminal is enough. `docker compose down` removes the container, `docker compose up -d` brings it back in a few seconds.

---

## 2. Reference

### Shortcuts

Add this to `C:\Users\david\.ssh\config` and connecting becomes `ssh oslab`:

```
Host oslab
    HostName localhost
    Port 2222
    User david
    IdentityFile ~/.ssh/id_ed25519
```

VS Code's **Remote - SSH** extension reads the same file: pick `oslab`, open `/home/david/OS`, and the editor, terminal and debugger all run inside Linux.

A shell without SSH, from any Windows terminal:

```bash
docker exec -it -u david -w /home/david/OS oslab bash
```

`-u david` matters: without it you are root. In Git Bash prefix the command with `MSYS_NO_PATHCONV=1`, otherwise Git Bash rewrites `/home/david/OS` into a Windows path.

### Tools inside

| Tool | For |
|---|---|
| `gcc`, `make`, `gdb` | compile and debug C (`gdb ./Z3`, then `run args`, `bt` after a crash) |
| `strace ./prog` | see every system call a program makes, great for pipes and fork |
| `python3` | the `.py3` scripts |
| `shellcheck s.sh` | lint a shell script |
| `dos2unix f` | fix a file that was saved with Windows line endings |
| `pgrep`, `ps`, `kill`, `lsof` | process tasks |
| `man 2 fork`, `man 3 pthread_create` | the manual pages the exams assume |
| `sudo` | works without a password |

### Maintenance

| Situation | Command |
|---|---|
| Container not running | `docker compose up -d` |
| Changed `docker/Dockerfile` | `docker compose up -d --build` |
| Want a package permanently | add it to the `apt-get install` line in the Dockerfile and rebuild; `sudo apt install` inside works but is lost when the container is recreated |
| See what sshd logged | `docker compose logs` |
| Start from scratch | `docker compose down`, `docker rmi oslab`, `docker compose up -d --build` |
| After a rebuild ssh says *REMOTE HOST IDENTIFICATION HAS CHANGED* | `ssh-keygen -R "[localhost]:2222"` and connect again. The host key lives in the image, so a rebuild makes a new one. |
| Lab 1 homework task that starts firefox | no desktop in the container; test it on the lab PC, or replace `firefox` by `sleep 100` to exercise the `pgrep`/`kill` logic |

### What it cannot do

- **Kernel modules (the driver tasks).** Containers borrow the kernel, they cannot change it (part 3 explains why). Those tasks are compile-by-reading only.
- **Graphics.** No desktop, no browser.
- **`systemd` services.** The container runs exactly one program, sshd.

---

## 3. How this is possible

### A container is not a small virtual machine

A virtual machine (what VirtualBox did) emulates a whole computer: virtual CPU, virtual disk, virtual network card, and inside it boots a complete operating system with its own kernel. That is why it takes a minute to start, holds gigabytes of RAM, and feels slow: two operating systems are running, one on top of the other.

A container is a group of ordinary processes on an **already running Linux kernel**, wrapped in isolation features the kernel itself provides:

| Kernel feature | What it isolates | Effect you see |
|---|---|---|
| PID namespace | process IDs | `ps` inside shows only the container's processes; sshd is PID 1 |
| mount namespace | the file system tree | `/` inside is the Ubuntu image, not the host's disk |
| network namespace | interfaces and ports | the container has its own port 22, mapped to the host's 2222 |
| UTS namespace | host name | `hostname` says `oslab` |
| IPC namespace | System V shared memory, semaphores | `shmget` keys inside do not clash with anything outside |
| user namespace | user IDs | `david` is uid 1000 inside, unrelated to Windows users |
| cgroups | CPU, memory, PID limits | the container cannot take the whole machine |

So when a program inside calls `fork()`, `pthread_create()`, `pipe()` or `shmget()`, it is a **real system call into a real Linux kernel**. Nothing is emulated. That is also why it starts in a second: there is no boot, just a process start.

### Where does Windows get a Linux kernel from?

From **WSL2**, Windows Subsystem for Linux. Microsoft ships a real Linux kernel (you saw it: `uname -r` reports `6.6.87.2-microsoft-standard-WSL2`) and runs it in a lightweight, hidden utility VM on Hyper-V. There *is* one virtual machine on your PC, but it is a single, minimal one: no desktop, no disk image to install, memory grows and shrinks on demand, and it boots in about two seconds. Docker Desktop keeps its own WSL2 distribution called `docker-desktop` (the one you saw in `wsl -l -v`) that holds the Docker engine, and every container you run is a process group inside that one VM.

Compared with the deleted VirtualBox machine, which had 11 GB of RAM assigned and a 13.7 GB disk with a full Ubuntu desktop, the difference in weight is the whole story.

### Image versus container

`docker/Dockerfile` is a recipe. `docker compose build` runs it once and produces an **image**: a stack of read-only layers, one per instruction. Ubuntu 24.04 base, then the apt packages, then the `david` user, then the sshd settings. Layers are content-addressed and shared, so ten images based on Ubuntu share one copy of Ubuntu.

A **container** is an image plus a thin writable layer plus one running process. Files you create or packages you install land in the writable layer, which is thrown away when the container is recreated (`docker compose down`). That is why anything permanent belongs in the Dockerfile, and why the repo is *mounted* into the container rather than copied into the image: a copy would be frozen at build time and would be lost with the writable layer.

The one running process here is `sshd`. `entrypoint.sh` does two things at every start: installs your public key as `authorized_keys` with the strict owner and permissions sshd demands, then `exec`s sshd in the foreground. `exec` replaces the shell with sshd, so sshd becomes PID 1; when it exits the container stops, and `restart: unless-stopped` in the compose file brings it back when Docker Desktop starts.

### The shared folder (bind mount)

`- .:/home/david/OS` in `docker-compose.yml` is a **bind mount**: a folder on the host made visible at a path inside the container's mount namespace. Nothing is copied. Docker Desktop exposes your NTFS folder to the WSL2 VM over a file-sharing protocol (9P), and Docker then mounts that into the container. Three consequences you will notice:

1. **A save on Windows is visible in Linux immediately**, and a file written by a program in Linux appears in Explorer immediately. There is exactly one copy.
2. **Permissions look odd.** `ls -l` shows everything as `rwxrwxrwx root root`. NTFS has no Unix permission bits, so the bridge reports a fixed set. `chmod` has no effect on files in the mount, which is harmless for the course.
3. **Line endings matter.** Linux bash reads the bytes as they are on disk. A script saved with Windows `CRLF` fails with `$'\r': command not found`. The repo's `.gitattributes` forces `LF` for `*.sh` and `*.py3`, and `dos2unix` fixes stragglers.

This is also why sshd's key file could not simply be mounted: sshd refuses an `authorized_keys` that is world-writable, and every file in a Windows mount is. The entrypoint copies the key to a native Linux path with `600` instead.

### How `ssh localhost:2222` reaches the container

Your ssh client connects to `127.0.0.1:2222` on Windows. Docker Desktop listens there and forwards the connection into the WSL2 VM, where Docker's network layer hands it to port 22 inside the container's network namespace, where sshd answers. The `127.0.0.1:` prefix in `ports:` means only programs on your own PC can reach it; without it the box, with its password `os`, would be reachable from the whole Wi-Fi network.

### Why `cap_add: SYS_PTRACE` and `seccomp=unconfined`

Docker strips containers of most Linux capabilities and filters system calls with seccomp, as a safety default. `ptrace`, the system call debuggers use, is among the blocked ones. Those two lines give `gdb` and `strace` their tools back. The container is local and runs your own code, so this costs nothing.

### Why kernel modules do not work

A driver task compiles a `.ko` file against the headers of the running kernel and loads it with `insmod`. Inside a container the "running kernel" is the WSL2 kernel shared by Docker Desktop and every container. Containers deliberately do not get the `SYS_MODULE` capability, because a module runs with full kernel privilege and would affect every container at once. On top of that, Microsoft does not ship headers for its kernel through `apt`. A full VM with its own kernel is the only place a module can be built and loaded, which is the one thing that was lost with VirtualBox. For the exam, reading and writing driver code is possible here; only loading it is not.

### Glossary

| Term | Meaning |
|---|---|
| Image | read-only, layered file system built from a Dockerfile; a template |
| Container | a running (or stopped) instance of an image with its own writable layer and namespaces |
| Dockerfile | the build recipe for an image |
| Compose file | `docker-compose.yml`, declares how to run containers: ports, mounts, restart policy |
| Layer | one read-only slice of an image, produced by one Dockerfile instruction |
| Bind mount | a host folder made visible inside the container; one copy of the files |
| Volume | like a bind mount, but the storage lives inside Docker (not used here) |
| Namespace | kernel mechanism that gives a process group its own view of PIDs, mounts, network, hostname, IPC or users |
| cgroup | kernel mechanism that limits and accounts CPU, memory and process counts for a group |
| WSL2 | Windows feature that runs a real Linux kernel in a lightweight Hyper-V VM |
| Entrypoint | the one program a container runs; here `entrypoint.sh`, which ends by `exec`ing `sshd` |
| Capability | a slice of root's power on Linux (`SYS_PTRACE`, `SYS_MODULE`, ...); containers get a reduced set |
