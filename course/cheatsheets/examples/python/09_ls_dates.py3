# python.md §5 — ls -al lines: month and day are fields 5 and 6; distinct dates (-d) and counts (-f)
from collections import defaultdict
ls_out = """total 20
drwxr-xr-x  2 david david 4096 Nov  3 15:17 .
drwxr-xr-x 10 david david 4096 Oct 28 09:00 ..
-rw-r--r--  1 david david  120 Nov  3 15:17 a.txt
-rw-r--r--  1 david david   88 Nov  3 16:01 b.txt
-rw-r--r--  1 david david   10 Oct 28 09:30 my file.txt
"""
count = defaultdict(int)
for line in ls_out.splitlines():
    f = line.split()
    # fields: 0 perms 1 links 2 owner 3 group 4 size 5 month 6 day 7 time 8.. name
    if len(f) < 9:                         # skips the "total 20" line
        continue
    count[f"{f[6]} {f[5]}"] += 1           # "3 Nov"; a name with spaces only adds fields after 8
print("-d:")
for d in count:                            # dict keeps order of first appearance
    print(d)
print("-f:")
for d, n in sorted(count.items()):         # pairs sort by the text first
    print(d, n)
