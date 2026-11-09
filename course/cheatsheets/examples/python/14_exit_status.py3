# python.md §7 — sys.exit codes consumed by a shell script (exams chain shell -> python)
import os
import subprocess
import sys
import tempfile

os.chdir(tempfile.mkdtemp())
CHILD = (
    "import sys\n"
    "if len(sys.argv) != 2:\n"
    "    print('Upotreba', file=sys.stderr)\n"
    "    sys.exit(2)\n"
    "n = len(open(sys.argv[1]).read().split())\n"
    "print(n)\n"                                   # the shell captures this with $( )
    "sys.exit(0 if n > 0 else 1)\n")
with open("proveri.py3", "w") as f:
    f.write(CHILD)
with open("prazna.txt", "w") as f:
    f.write("")
with open("polna.txt", "w") as f:
    f.write("a b c\n")
shell = r'''
for f in polna.txt prazna.txt; do
    n=$(python3 proveri.py3 "$f")
    echo "$f: output=$n status=$?"
done
python3 proveri.py3; echo "no args: status=$?"
if python3 proveri.py3 polna.txt > /dev/null; then echo "if sees status 0 as true"; fi
'''
subprocess.run(["bash", "-c", shell])
sys.exit(0)
