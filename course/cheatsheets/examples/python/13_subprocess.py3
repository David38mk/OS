# python.md §7 — subprocess.run (capture, check, returncode), Popen in parallel, os.system
import os
import subprocess
import time

r = subprocess.run(["echo", "zdravo"], capture_output=True, text=True)
print(repr(r.stdout), r.returncode)               # stdout is text because of text=True
r = subprocess.run(["ls", "/nema/takvo/nesto"], capture_output=True, text=True)
print(r.returncode, r.stderr.strip())             # failure is a number, not an exception...
try:
    subprocess.run(["false"], check=True)         # ...unless check=True
except subprocess.CalledProcessError as e:
    print("check=True raised, code", e.returncode)
try:
    subprocess.run(["naredba-sto-ne-postoi"])
except FileNotFoundError:
    print("program not found: FileNotFoundError")

start = time.time()
procs = [subprocess.Popen(["sleep", "1"]) for _ in range(3)]   # all three start at once
for p in procs:
    p.wait()                                      # now wait for each
print("3 x sleep 1 took", round(time.time() - start), "s (parallel)")

status = os.system("exit 3")                      # runs through the shell, returns a wait status
print(status, os.waitstatus_to_exitcode(status))  # 768 = 3 * 256 on Linux
