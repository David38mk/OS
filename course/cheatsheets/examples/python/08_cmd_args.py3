# python.md §5 — command words followed by dash-arguments that may continue on the next lines
import os
import tempfile

os.chdir(tempfile.mkdtemp())
with open("naredbi.txt", "w") as f:
    f.write("ls\ncp\nrm\n")
with open("tekst.txt", "w") as f:
    f.write("ls -l -a\n-h file.txt cp -r\nsrc dst ls\n-R rm\n")

with open("naredbi.txt") as f:
    komandi = f.read().split()                    # one command per line
with open("tekst.txt") as f:
    zborovi = f.read().split()                    # newlines vanish: arguments span lines for free
rez = {}                                          # command -> arguments (dict keeps first-seen order)
i = 0
while i < len(zborovi):
    z = zborovi[i]
    if z in komandi:
        args = []
        i += 1
        while i < len(zborovi) and zborovi[i].startswith("-"):   # stop at first non-dash word
            args.append(zborovi[i])
            i += 1
        if z not in rez:
            rez[z] = args
        elif args:
            rez[z] += ["-e"] + args               # repeat: add to the first one, after "-e"
    else:
        i += 1
for k, v in rez.items():
    print(k, " ".join(v))
