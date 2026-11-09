# python.md §1 — argv skeleton: required arg, optional number, flag, usage to stderr, exit codes
import sys

def usage():
    print(f"Upotreba: {sys.argv[0]} <datoteka> [N] [-b]", file=sys.stderr)
    sys.exit(1)

if len(sys.argv) < 2:          # argv[0] is the script name, so 2 means one real argument
    usage()

ime = sys.argv[1]
N = 10                         # default used when the optional argument is missing
blok = False
for arg in sys.argv[2:]:       # everything after the file name, in any order
    if arg == "-b":
        blok = True
    elif arg.isdigit():
        N = int(arg)
    else:
        print(f"Nepoznat argument: {arg}", file=sys.stderr)
        sys.exit(2)

print(f"datoteka={ime} N={N} -b={blok}")
print("argv =", sys.argv)
sys.exit(0)                    # 0 = success, any other number = failure
