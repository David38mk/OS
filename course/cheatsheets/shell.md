# Bash Shell Cheat Sheet – study guide for the OS labs and exams

A reference and a brush-up manual in one. Read sections 0 to 8 in order once (they explain how the shell thinks) and section 10 before you touch `grep`, `sed` or `awk`, then use the rest as a lookup. Every example is written the way the course tasks need it: Linux, `bash`, GNU tools. Lines starting with `$` are typed at the prompt, lines without it are output.

Where a recipe comes straight from an exam or lab in this repo, the folder is named so you can open the full solution.

---

## 0. How the shell reads a command line

Before anything runs, bash transforms the line you typed. Knowing the order explains almost every "why did that break" moment:

1. **Split into words** on spaces and tabs.
2. **Expand**: brace `{a,b}`, tilde `~`, variables `$x`, command substitution `$(cmd)`, arithmetic `$(( ))`.
3. **Word splitting** of the expansion results (again on spaces, tabs, newlines).
4. **Globbing**: words containing `* ? [` are replaced by matching file names.
5. **Quote removal**: the quotes themselves are deleted.
6. The command is run with the final list of words as its arguments.

The command never sees your quotes or your `*`. `grep *.c` does not pass `*.c` to grep: bash first replaces `*.c` with `a.c b.c` and grep receives those. That is why patterns meant for another program must be quoted (section 3 and 4).

**Exit status.** Every command ends with a number: `0` means success, anything else means failure. `$?` holds the status of the last command.

```bash
$ grep -q password file.py; echo $?      # 0 if found, 1 if not, 2 on error (file missing)
$ if grep -q password file.py; then echo "ima"; fi   # if tests the status directly, no [ ] needed
$ mkdir x && cd x          # run cd only if mkdir succeeded
$ cd /nope || exit 1       # exit the script if cd failed
```

---

## 1. Running scripts

```bash
#!/bin/bash              # first line: which interpreter runs the file ("shebang")
```

| Way to run | Needs `chmod +x`? | Runs in |
|---|---|---|
| `./skripta.sh a b` | yes | new bash process |
| `bash skripta.sh a b` | no | new bash process |
| `source skripta.sh` or `. skripta.sh` | no | the current shell (its variables and `cd` stay) |

Useful while developing:

| Command | Does |
|---|---|
| `bash -n s.sh` | syntax check only, nothing runs |
| `bash -x s.sh` | prints every command with its expanded values as it runs |
| `set -x` / `set +x` | turn that tracing on/off inside the script |
| `set -e` | exit on the first failing command |
| `set -u` | error on an unset variable instead of silently using "" |
| `shellcheck s.sh` | linter that catches quoting and `[ ]` mistakes |

**Windows line endings.** A script saved with CRLF fails on Linux with `$'\r': command not found` or `bad interpreter`. Fix: `dos2unix s.sh` or `sed -i 's/\r$//' s.sh`. This repo forces LF through `.gitattributes`.

---

## 2. Files and directories

| Command | Most used flags | Example |
|---|---|---|
| `ls` | `-l` long, `-a` hidden too, `-1` one per line, `-R` recursive, `-h` sizes, `-t` by time, `-S` by size | `ls -la`, `ls -1 \| grep '\.c$'` |
| `cd` | `cd -` previous dir, `cd` alone = home, `cd ..` | `cd "$dir" \|\| exit 1` |
| `pwd` | | `here=$(pwd)` |
| `mkdir` | `-p` create parents, no error if exists | `mkdir -p out/2026` |
| `touch` | creates empty file or updates its time | `touch a.txt` |
| `cp` | `-r` directories, `-i` ask, `-n` never overwrite, `-p` keep times/permissions | `cp "$f" .` |
| `mv` | rename or move; `-i`, `-n` | `mv old.txt new.txt` |
| `rm` | `-r` directories, `-f` no questions, `-i` ask | `rm "./$name"` |
| `rmdir` | only empty directories | |
| `cat` | `-n` numbers | `cat "$f"`, `cat a b > ab` |
| `less` | pager; `/` search, `q` quit | `less big.log` |
| `head` / `tail` | `-n 5` (or `-5`), `tail -f` follow | `head -1`, `tail -n +2` (skip first line) |
| `wc` | `-l` lines, `-w` words, `-c` bytes, `-m` chars | `grep -c` is often simpler than `grep \| wc -l` |
| `file` | guess the type by content | `file x.bin` |
| `stat` | size, times, permissions | `stat -c %s f` (size), `stat -c %a f` (perms as 644) |
| `du` / `df` | `-sh` dir size / `-h` free space | `du -sh *` |
| `ln` | `-s` symbolic link | `ln -s target linkname` |
| `basename` / `dirname` | last part / everything before it | `basename a/b/c.txt` is `c.txt`, `basename c.txt .txt` is `c` |
| `realpath` | absolute path | |
| `which` / `type` / `command -v` | where a command lives | `command -v python3` |
| `tree` | directory tree | `tree -L 2` |

Current directory is `.`, parent is `..`, home is `~` (or `$HOME`).

---

## 3. Wildcards (globs, „џокер знаци“)

Patterns that bash expands to **file names** that exist. If nothing matches, the pattern is left as-is (literal text).

| Pattern | Matches | Example |
|---|---|---|
| `*` | any string, including empty, but not a leading `.` and never `/` | `*.c` → `a.c lab5.c`; `lab*` → `lab4.c lab5.c labs/` |
| `?` | exactly one character | `lab?.c` → `lab4.c` but not `lab41.c` |
| `[abc]` | one character from the set | `lab[456].c` |
| `[a-z]`, `[0-9]` | one character from the range | `Z[1-3].sh` |
| `[!abc]` or `[^abc]` | one character **not** in the set | `[!.]*` everything not starting with a dot |
| `[[:digit:]]`, `[[:alpha:]]`, `[[:upper:]]`, `[[:space:]]` | POSIX classes inside brackets | `*[[:digit:]].c` |
| `{a,b,c}` | **brace expansion**, not a glob: produces every alternative whether files exist or not | `cp f.txt{,.bak}` → `cp f.txt f.txt.bak`; `mkdir -p d/{src,tests}`; `echo {1..5}` → `1 2 3 4 5` |
| `**` | any depth of directories, only after `shopt -s globstar` | `ls **/*.c` |
| `~` | home directory | `~/OS` |

Behaviour to remember:

- **Hidden files** (`.bashrc`) are not matched by `*`. Use `.*` (which also matches `.` and `..`) or `shopt -s dotglob`.
- **No match**: `echo *.xyz` prints `*.xyz` literally. `shopt -s nullglob` makes it expand to nothing instead; `shopt -s failglob` makes it an error. In loops over `*.txt` always guard with `[ -e "$f" ] || continue`.
- Globs match the **whole file name**: `*.c` is "ends with .c", there is no implicit "contains".
- Globbing happens **after** variable expansion and only on **unquoted** text. `"*.c"` or `'*.c'` stays literal.

### Glob vs. regular expression

They look alike and mean different things. Globs are matched by the shell against file names; regexes are matched by `grep`, `sed`, `awk` and `[[ =~ ]]` against text. Section 10 has the symbol-by-symbol comparison and says which program interprets what.

| Meaning | Glob (shell, `find -name`, `case`) | Regex (`grep`, `sed`, `awk`) |
|---|---|---|
| any string | `*` | `.*` |
| any single char | `?` | `.` |
| repeat previous item | not possible | `*` (0+), `+` (1+), `?` (0 or 1), `{2,4}` |
| set / range | `[abc]`, `[a-z]` | same |
| anchored to the whole thing | always | only with `^` and `$` |
| literal dot | `.` | `\.` |
| "ends with .c" | `*.c` | `\.c$` |

So `ls *.c` and `ls | grep '\.c$'` list the same files. `grep *.c` would be a bug (section 0), and `grep .c file` matches any character followed by `c`.

### Quote globs meant for another program

`find -name` does its own matching and expects the pattern as text:

```bash
$ find . -name '*.c'      # correct: find receives *.c
$ find . -name *.c        # wrong: if lab5.c exists in the current dir, bash turns this into
                          #        find . -name lab5.c, and with two .c files find errors out
```

Same for `case` patterns (never quoted, they are globs by definition) and `[[ $f == *.py ]]` (right side unquoted = glob match, quoted = literal compare).

---

## 4. Quoting: single, double, none

The most common source of bugs in scripts. What each form does:

| | `'single'` | `"double"` | no quotes |
|---|---|---|---|
| `$var`, `${var}` | literal | expanded | expanded |
| `$(cmd)`, `` `cmd` `` | literal | expanded | expanded |
| `$(( 1+2 ))` | literal | expanded | expanded |
| `\` escapes | none (`\` is literal) | only `\$ \" \\ \`` and `\newline` | all |
| globbing (`*`) | no | no | yes |
| word splitting on spaces | no | no | yes |
| can contain | anything but `'` | anything, escape `"` as `\"` | no spaces |

**Single quotes: everything is literal.** Use them for anything another program will interpret: regexes, sed and awk programs, find patterns.

```bash
$ grep '\.c$' list.txt
$ sed 's/\r$//' file
$ awk '{print $1}' file        # $1 is awk's field, bash must not touch it
$ find . -name '*.py3'
```

A single quote cannot appear inside single quotes. Either switch to double quotes or close, escape, reopen: `'it'\''s'`.

**Double quotes: variables and command substitution expand, nothing else happens.** Use them around every variable and `$(...)`, always.

```bash
$ f="my file.txt"
$ cat "$f"          # one argument: my file.txt
$ cat $f            # two arguments: "my" and "file.txt"  -> two errors
$ n="$(wc -l < "$f")"
$ echo "Najdeni: $n datoteki vo $(pwd)"
```

Expansion inside double quotes also keeps newlines, so `echo "$(ls)"` prints one name per line while `echo $(ls)` prints them all on one line.

**No quotes: word splitting and globbing happen.** Occasionally wanted (`for f in *.txt`), usually a bug. An unquoted empty variable disappears entirely: `[ $x = "a" ]` with empty `x` becomes `[ = "a" ]` and errors.

**Mixing.** When a pattern needs a variable, put the variable in double quotes and the rest in single quotes, or pass it in properly:

```bash
$ grep "$word" file                 # whole pattern is the variable
$ sed "s/stara/$nova/" file         # variable inside sed program: double quotes
$ sed "s#/stara/pateka#$nova#" f    # path contains /, so use # as the delimiter
$ awk -v w="$word" '$1 == w' file   # the clean way to get a shell variable into awk
$ echo 'Cena: '"$cena"' denari'     # literal parts in single, variable in double, no spaces between
```

**`$'...'` (ANSI-C quoting)** turns escape sequences into real characters: `$'\n'` newline, `$'\t'` tab. Handy for `IFS=$'\n'`.

### `$@` vs `$*`

With the script called as `./s.sh "a b" c`:

| Form | Expands to | Use |
|---|---|---|
| `"$@"` | `"a b"` `c` (two words, as given) | **always this** when passing or looping over arguments |
| `$@` or `$*` | `a` `b` `c` (split again) | almost never |
| `"$*"` | `"a b c"` (one word, joined with a space) | only for printing all args as one string |

```bash
for name in "$@"; do ...; done     # or just:  for name; do ...; done
```

---

## 5. Variables, arguments, arithmetic, strings

### Variables

```bash
x=5                 # no spaces around =
name="David N"      # quote values with spaces
echo "$x ${name}"   # ${ } needed when text follows directly: "${x}km"
unset x
readonly PI=3.14
export PATH="$PATH:$HOME/bin"    # visible to child processes
```

Variables are strings. `x=5; y=$x+1` gives `5+1`, see arithmetic below.

### Script arguments (positional parameters)

| | Meaning |
|---|---|
| `$0` | script name |
| `$1` … `$9`, `${10}` | arguments |
| `$#` | number of arguments |
| `"$@"` | all arguments, kept separate |
| `$?` | exit status of the last command |
| `$$` | PID of this shell |
| `$!` | PID of the last background process |
| `shift` | drop `$1`, everything moves down (`shift 2` drops two) |

The exam opener, in every script:

```bash
if [ "$#" -lt 1 ]; then
    echo "Nema argumenti"
    exit 1
fi
```

Defaults and required values:

```bash
name="${1:-naredbi.txt}"    # $1 if given and non-empty, else naredbi.txt   (Lab 1 2026, Z2)
file="${1:?Nedostasuva ime na datoteka}"   # print message and exit 1 if $1 missing
```

### Parameter expansion (string surgery without external commands)

| Expression | Result for `f=dir/sub/lab5.tar.gz` |
|---|---|
| `${#f}` | length, `19` |
| `${f##*/}` | `lab5.tar.gz` (basename: remove longest prefix up to last `/`) |
| `${f%/*}` | `dir/sub` (dirname: remove shortest suffix from last `/`) |
| `${f%.*}` | `dir/sub/lab5.tar` (remove last extension) |
| `${f%%.*}` | `dir/sub/lab5` (remove all extensions) |
| `${f##*.}` | `gz` (the extension) |
| `${f#dir/}` | `sub/lab5.tar.gz` (remove prefix) |
| `${f/lab/LAB}` | first `lab` replaced; `${f//a/A}` all |
| `${f:4:3}` | `sub` (substring: offset 4, length 3) |
| `${f^^}` / `${f,,}` | upper / lower case |

Rule: `#` cuts from the front (it is left of `$` on the keyboard), `%` from the back. One symbol = shortest match, two = longest.

### Arithmetic

Only integers in bash:

```bash
total=$(( total + 1 ))      # or (( total++ ))
procent=$(( found * 100 / total ))      # 2 of 3 -> 66, integer division  (Lab 1 2026, Z1)
if (( a > b && b != 0 )); then ...; fi   # C-like comparison inside (( ))
let x=x+1                   # older form
expr 3 + 4                  # external command, old style; spaces required
```

Decimals need another program:

```bash
echo "scale=2; $a * 100 / $b" | bc           # 66.66
awk -v a="$a" -v b="$b" 'BEGIN { printf "%.2f%%\n", a*100/b }'
printf "%.1f\n" "$(echo "$x/3" | bc -l)"
```

### Arrays

```bash
arr=(a b c)                       # indexed array
arr+=(d)                          # append
echo "${arr[0]}" "${arr[-1]}"     # first, last
echo "${arr[@]}"                  # all elements, as separate words
echo "${#arr[@]}"                 # count
for x in "${arr[@]}"; do ...; done

z=( $(find . -type f -name '*.ip') )   # as in the 20.11.2023 exam: works while names have no spaces
mapfile -t z < <(find . -type f -name '*.ip')   # safe version, one line per element

declare -A broj                   # associative array (map)
broj[BTC]=3; echo "${broj[BTC]}"; echo "${!broj[@]}"   # keys
```

### `read`

```bash
read -r line                        # one line from stdin into $line; -r keeps backslashes literal
read -p "Vnesi broj: " n            # with prompt
IFS=: read -r user _ uid _ <<< "$line"   # split a line on : into variables
```

---

## 6. Pipes, redirection, command substitution

### Redirection

| Syntax | Meaning |
|---|---|
| `cmd > f` | stdout to file, **overwrite** |
| `cmd >> f` | stdout to file, append |
| `cmd < f` | stdin from file |
| `cmd 2> f` | stderr to file |
| `cmd > f 2>&1` | both to file (order matters: first redirect stdout, then send stderr where stdout goes) |
| `cmd &> f` | same, bash shorthand |
| `cmd 2>/dev/null` | discard errors |
| `cmd > /dev/null 2>&1` | discard everything |
| `cmd <<< "text"` | here-string: text as stdin |
| `cmd << 'EOF' … EOF` | here-document: lines as stdin. Quote `'EOF'` to keep `$` literal |
| `cmd \| tee f` | stdout to screen **and** file (`tee -a` appends) |

Never read and write the same file in one pipeline (`sort f > f` empties `f` first). Write to a temp file and `mv`.

### Pipes

`a | b` connects a's stdout to b's stdin. `|&` also pipes stderr. The exit status of a pipeline is the last command's unless `set -o pipefail`.

```bash
grep -E '^[0-9]{2}-[0-9]{2}-[0-9]{4}' "$out" | wc -l     # count lines that start with a date  (2019/20 exam)
ls -1 | grep -E '\.c$'
find . -name '*.txt' | head -1
sort f | uniq -c | sort -rn | head -5          # top 5 most frequent lines
```

### Command substitution

```bash
n=$(wc -l < file)            # preferred form, nests cleanly: $(basename "$(pwd)")
n=`wc -l < file`             # old form, avoid
content=$(< file)            # whole file into a variable (bash shortcut for $(cat file))
result=$(./Z2.py3 "$f")      # capture another program's output   (21.11.2024 exam)
```

Trailing newlines are removed from the result. Quote it when you use it: `echo "$result"`.

### Process substitution

`<(cmd)` gives a command's output a file name:

```bash
diff <(sort a.txt) <(sort b.txt)
while IFS= read -r -d '' f; do ...; done < <(find . -name '*.py' -print0)
```

The second form matters: `find … | while read f; do count=$((count+1)); done` runs the loop in a **subshell**, so `$count` is still 0 afterwards. Feeding the loop with `< <(…)` keeps it in the current shell.

### xargs

Turns stdin lines into arguments:

```bash
find . -name '*.log' -print0 | xargs -0 rm -f           # -0 pairs with -print0, safe for spaces
grep -l password *.py | xargs -I {} cp {} backup/        # -I {} puts each item where {} is
echo a b c | xargs -n 1 echo                              # one item per command
```

---

## 7. Conditions: `test`, `[ ]`, `[[ ]]`, `if`, `case`

`[` is a command (same as `test`), so **spaces around everything are mandatory**: `[ "$a" = "$b" ]`, not `["$a"="$b"]`. `[[ ]]` is a bash keyword with saner rules: no word splitting, no globbing on the left, `&&`/`||` inside, pattern and regex matching.

| Test | True when |
|---|---|
| `-e f` | exists (any type) |
| `-f f` | regular file |
| `-d f` | directory |
| `-s f` | exists and not empty |
| `-r f` / `-w f` / `-x f` | readable / writable / executable |
| `-L f` | symbolic link |
| `f1 -nt f2` / `-ot` | newer / older than |
| `-z s` | string is empty |
| `-n s` | string is not empty (`[ "$s" ]` is the same) |
| `s1 = s2` / `!=` | strings equal / differ (`==` also works in bash) |
| `s1 < s2` | sorts before (inside `[ ]` escape as `\<`) |
| `n1 -eq n2` | integers equal; also `-ne -lt -le -gt -ge` |
| `! expr` | not |

Never use `<`, `>`, `==` for numbers in `[ ]`: `[ 10 \> 9 ]` is a string compare and is **false**. Use `-gt`, or `(( 10 > 9 ))`.

```bash
if [ -f "./$name" ] && [ -n "$sub" ]; then       # two tests joined with && (preferred over -a)
    rm "./$name"
elif [ -f "./$name" ]; then
    cat "./$name"
else
    cat "$sub"
fi

if [[ "$f" == *.py || "$f" == *.py3 ]]; then ...; fi      # glob match, right side unquoted
if [[ "$ip" =~ ^([0-9]{1,3}\.){3}[0-9]{1,3}$ ]]; then ...; fi   # regex, unquoted; groups in ${BASH_REMATCH[1]}
if [ -z "$(find . -name "$name" | head -1)" ]; then echo "Ne e pronajdena"; fi
```

The one-liner `[ -f x ] && echo da || echo ne` works, but `a && b || c` runs `c` also when `b` fails, so do not use it when `b` can fail.

### `case`

Glob patterns, first match wins, `;;` ends a branch:

```bash
case "$f" in
    *.py|*.py3)  echo "python" ;;
    *.sh)        echo "shell" ;;
    [0-9]*)      echo "pocnuva so cifra" ;;
    "")          echo "prazno" ;;
    *)           echo "drugo" ;;
esac
```

---

## 8. Loops

```bash
for f in *.txt; do [ -e "$f" ] || continue; echo "$f"; done   # over files (guard for no match)
for name in "$@"; do ...; done                                 # over script arguments
for i in 1 2 3; do ...; done
for i in {1..10}; do ...; done
for i in $(seq 1 "$n"); do ...; done                           # when the bound is a variable
for (( i = 0; i < n; i++ )); do ...; done                      # C style
for d in */; do echo "dir: ${d%/}"; done                       # directories only

while [ "$i" -lt 10 ]; do ...; i=$((i+1)); done
until [ -f done.flag ]; do sleep 1; done

while IFS= read -r line; do echo "-> $line"; done < file.txt   # file line by line
while IFS= read -r -d '' f; do cp "$f" .; done < <(find . -name '*.py' -print0)   # find results, safe

break      # leave the loop;  break 2 leaves two nested loops
continue   # next iteration
```

Do not loop over `$(ls)` or `$(find …)` when names can contain spaces: each word becomes an item. Use globs, `-print0` with `read -d ''`, or `find -exec`.

Count and accumulate:

```bash
found=0; total=0
for name in "$@"; do
    total=$((total + 1))
    if [ -n "$(find . -type f -name "$name" | head -1)" ]; then
        echo "$name - najdena"; found=$((found + 1))
    else
        echo "$name - ne e najdena"
    fi
done
echo "Procent na najdeni datoteki: $((found * 100 / total))%"       # Lab 1 2026, Z1
```

---

## 9. `find` in depth

`find [where] [tests] [actions]`. Tests are ANDed by default; `-o` is OR; `!` or `-not` negates; group with `\( \)` (escaped so bash does not treat them as a subshell).

| Test | Meaning |
|---|---|
| `-name '*.c'` | file name matches glob (quote it); `-iname` ignores case |
| `-path '*/tests/*'` | whole path matches |
| `-type f` / `d` / `l` | file / directory / symlink |
| `-maxdepth 1` | only the start directory (no recursion) |
| `-mindepth 2` | only inside subdirectories, not the start directory itself |
| `-size +1M`, `-size -10k`, `-size 0` | bigger / smaller / empty |
| `-empty` | empty file or directory |
| `-mtime -1`, `-mmin +30` | modified less than 1 day ago / more than 30 min ago |
| `-newer ref.txt` | modified after ref.txt |
| `-user david`, `-perm -u+x` | owner, permissions |

| Action | Meaning |
|---|---|
| `-print` | default; one path per line |
| `-print0` | NUL-separated, for `xargs -0` or `read -d ''` |
| `-exec cmd {} \;` | run cmd once per file (`{}` = the path) |
| `-exec cmd {} +` | run cmd once with all files (much faster) |
| `-ok cmd {} \;` | like -exec but asks first |
| `-delete` | delete (put it last!) |
| `-printf '%s %p\n'` | formatted output (size and path) |

Recipes from the exams and labs:

```bash
find . -type f -name naredbi.txt                     # anywhere below .          (19.11.2019, Lab 1 2026 Z2)
find . -mindepth 2 -type f -name naredbi.txt         # only in subdirectories
find . -maxdepth 1 -type f -name '*.c'               # only in the current dir
find . -type f \( -name '*.py' -o -name '*.py3' \)   # either extension           (Lab 1 2026 Z3)
find . -type f -name '*.crypto'                      # (21.11.2024)
find . -type f -name '*.py' -exec grep -l password {} +      # names of files containing the word
find . -type d -name '*.ecf'                         # directories by name        (30.01.2019)
find . -name '*.log' -size +10M -delete
find . -type f -newer ref.txt -print
n=$(find . -type f -name '*.ip' | wc -l)             # how many
first=$(find . -type f -name "$name" | head -1)      # first match, or empty
```

`find` prints paths starting with `./`. `dirname "$f"` is `.` exactly when the file is in the current directory, which is how "already here, skip it" is tested.

---

## 10. Patterns: who interprets what, and regular expressions in depth

Three different pattern languages show up on one command line, and the same symbol means different things in each. Getting this straight removes most of the confusion around `grep`, `sed` and `awk`.

### 10.1 Two layers: the shell goes first, the program goes second

Every command line is processed twice. **Bash** reads it first and handles quotes, `$variables`, globs, pipes and redirections (section 0). Only then does it start the program (`grep`, `sed`, `awk`, `find`) and hand it the remaining words. The program never sees your quotes; it sees the result. So the question for every special character is: *do I want bash to act on this, or do I want to deliver it untouched to the program?*

- Want bash to act (expand a variable, match file names, pipe): leave it unquoted, or in double quotes for variables.
- Want the program to get it untouched (a regex, a `find -name` pattern, an awk program): single quotes.

Trick to see what the program will actually receive: put `echo` in front of the command.

```bash
$ echo grep -E lab[0-9]+.c$ *.txt        # bash expanded *.txt, and would also expand lab[0-9]+.c$ if a file matched
grep -E lab[0-9]+.c$ a.txt b.txt
$ echo grep -E 'lab[0-9]+\.c$' *.txt     # the pattern arrives intact
grep -E lab[0-9]+\.c$ a.txt b.txt
```

### 10.2 The same symbol in the shell and in a regex

| Symbol | Bash, unquoted | Inside a regex (grep/sed/awk) | Safe way to send it to the program |
|---|---|---|---|
| `*` | glob: any characters **in a file name** (`*.c`) | repeat the previous item 0 or more times (`[0-9]*`) | single quotes |
| `?` | glob: one character in a file name | previous item optional (`colou?r`) | single quotes |
| `[abc]`, `[a-z]` | glob: one character from the set, in a file name | one character from the set, in the text | single quotes (same syntax, different target) |
| `[!abc]` / `[^abc]` | glob negation is `!` | regex negation is `^` | |
| `{a,b}`, `{1,3}` | **brace expansion**: `echo [0-9]{1,3}` prints `[0-9]1 [0-9]3` | repetition count: `[0-9]{1,3}` | single quotes, or the pattern silently breaks |
| `\|` | pipe between commands | ERE alternation (`BTC\|ETH`) | single quotes |
| `$` | start of a variable (`$x`, `$(cmd)`) | end of line (`\.c$`) | single quotes; in double quotes write `\$` |
| `^` | nothing special | start of line | |
| `( )` | subshell | group | single quotes |
| `\` | escape the next character | escape (`\.`) or special class (`\b`, `\<`) | single quotes; in double quotes `\.` survives but `\\` becomes `\` |
| `.` | nothing special | any single character | |
| `#` | start of a comment | literal; also a common `s#a#b#` delimiter in sed | single quotes |
| `&` | run in background | sed replacement: the whole match | single quotes |
| `<` `>` | redirection | GNU `\<` `\>` word start / end | single quotes |
| `;` | separates commands | separates sed commands (`'/^#/d; /^$/d'`) | single quotes |
| `~` | home directory | awk match operator (`$1 ~ /re/`) | single quotes |
| `!` | history expansion in an interactive shell, also inside `"..."` | nothing | avoid in double quotes at the prompt |
| space | separates arguments | literal space | quote a pattern that contains one |

Rule that follows from the table: **a regex goes in single quotes, always.** Switch to double quotes only for the part that must contain a variable, and escape `$` there if it means end-of-line: `grep -E "^$name:.*\$"` or, cleaner, `grep -E '^'"$name"':.*$'`.

### 10.3 Three pattern languages and where each one is used

| Language | Matches against | Used by | Example |
|---|---|---|---|
| **Glob** | file names, or a whole string | bash command line, `case`, `[[ $s == pat ]]`, `${var#pat}` `${var%pat}` `${var/pat/rep}`, `find -name` | `*.c`, `Z[1-3].sh`, `${f%.*}` |
| **Regex** | text, anywhere inside a line unless anchored | `grep`, `sed`, `awk`, `[[ $s =~ re ]]`, `find -regex`, `expr` | `^[0-9]+$`, `\.c$` |
| **Fixed string** | exact characters, nothing special | `grep -F`, awk `index()`, bash `==` with a quoted right side | `grep -F '1.5'` |

The surprise in that table: bash's own string operators use **glob** syntax, not regex. `${f%.*}` removes the shortest suffix matching the glob `.*` (a dot and anything), `${s//[0-9]/}` deletes digits with a glob set, and `case $x in *.py)` is a glob. Only `[[ =~ ]]` uses a regex. A glob always has to match the **whole** string, a regex matches a **part** unless you write `^` and `$`.

### 10.4 Which tool for which job

| I want to... | Use | Pattern type |
|---|---|---|
| pick files by name in this directory | a glob: `for f in *.c` | glob |
| pick files by name anywhere below | `find . -name '*.c'` (or `-regex` for complex names) | glob (regex with `-regex`) |
| know whether a string or variable has a shape | `[[ $x =~ ^[0-9]+$ ]]` | regex |
| branch on the shape of a string | `case "$x" in *.py\|*.py3) ...` | glob |
| strip or replace part of a variable | `${f%.c}`, `${f##*/}`, `${s/old/new}` | glob |
| find the lines that contain something | `grep -E` | regex |
| know if a file contains something (for `if`) | `grep -qE` | regex |
| pull the matching pieces out of text | `grep -oE`, or `sed -E 's/.*(group).*/\1/'` | regex |
| change text in a stream or a file | `sed -E 's/.../.../'`, `sed -i` | regex |
| work with columns, numbers, sums | `awk` | fields; regex to select rows |
| count lines that match | `grep -cE`, or `awk '/re/ {n++} END {print n}'` | regex |
| search for text with dots or stars in it literally | `grep -F`, `awk 'index($0, w)'` | fixed string |

### 10.5 Three regex dialects

| Where | Dialect | What it means for you |
|---|---|---|
| `grep`, `sed` without `-E` | BRE, basic | `+ ? \| ( ) { }` are **literal**. For the special meaning write `\+ \? \| \( \) \{ \}` |
| `grep -E`, `sed -E`, `awk`, bash `[[ =~ ]]` | ERE, extended | the special characters work bare; `\(` is a literal parenthesis |
| `grep -P` | PCRE, Perl | adds `\d \w \s`, lazy `*?`, lookahead. Not available in sed or awk |

Use `-E` everywhere and you only need to learn one syntax. All examples below assume it. Two traps that follow:

- **`\d` does not exist** in ERE. On the lab machines' GNU grep 3.11 `grep -E '\d'` quietly matches a literal `d`. Write `[0-9]` or `[[:digit:]]`.
- GNU grep and GNU sed add `\b \< \> \w \W \s \S`. Ubuntu's default `awk` is **mawk**, which supports **none** of them. In awk use `[[:space:]]`, `[[:alnum:]_]` and compare whole fields instead of word boundaries.

### 10.6 Building blocks, each with a runnable example

**Literal text, and escaping the specials.** In ERE these are special: `. * + ? [ ] ( ) { } | ^ $ \`. A backslash makes them literal. Inside `[...]` only `] \ ^ -` are special.

```bash
echo 'cena: $5.00' | grep -E '\$5\.00'      # escape $ and .  (unescaped . would also match "5x00")
echo 'cena: $5.00' | grep -F '$5.00'        # or -F: nothing is special
grep -E '\.c$' list.txt                      # names ending in .c
grep -E '\[ERROR\]' app.log                  # a literal [ERROR]
```

**`.` any single character.**

```bash
printf 'lab\nlib\nl5b\nlb\n' | grep -E 'l.b'   # lab lib l5b   (not lb: exactly one character)
grep '192.168.1.1' hosts                       # BUG: also matches 192x168-1-1
grep -E '\b192\.168\.1\.1\b' hosts             # correct
```

**Character sets `[...]`.**

| Set | Meaning |
|---|---|
| `[abc]` | one of a, b, c |
| `[a-z]`, `[0-9]`, `[a-zA-Z0-9]` | ranges |
| `[^0-9]` | one character that is **not** a digit |
| `[]abc]`, `[a-]` | a literal `]` goes first, a literal `-` goes last |
| `[[:digit:]]` `[[:alpha:]]` `[[:alnum:]]` `[[:upper:]]` `[[:lower:]]` `[[:space:]]` `[[:punct:]]` `[[:xdigit:]]` | POSIX classes, used **inside** the brackets |

POSIX classes follow the locale: `[[:alpha:]]` matches `Скопје`, `[a-zA-Z]` does not.

```bash
grep -E '^[A-Z]' names.txt                 # starts with a capital
grep -E '^[[:alpha:]]+$' words.txt         # letters only, Cyrillic included
echo 'Z1 Z2 z3' | grep -oE 'Z[0-9]'        # Z1 Z2
grep -nE '[[:space:]]$' s.sh               # lines with trailing whitespace
grep -E '[^[:print:]]' f                   # lines with control characters (a CR, for example)
```

**Anchors `^` and `$`.** Without them a regex matches anywhere in the line.

```bash
grep -E '^#' s.sh              # comment lines
grep -cE '^$' f                # how many empty lines
grep -E '^[0-9]+$' f           # the whole line is a number
grep -E '\.py3?$' list         # ends with .py or .py3
sed -E 's/^/    /' f           # insert at the start of every line (indent)
sed -E 's/$/;/' f              # append to every line
```

**Quantifiers** apply to the item just before them.

| `*` | `+` | `?` | `{3}` | `{2,}` | `{1,3}` |
|---|---|---|---|---|---|
| 0 or more | 1 or more | 0 or 1 | exactly 3 | 2 or more | 1 to 3 |

```bash
grep -oE '[0-9]+' f                          # every run of digits
grep -E '^[0-9]{2}-[0-9]{2}-[0-9]{4}' f      # starts with dd-mm-yyyy
echo 'colour color colr' | grep -oE 'colou?r'   # colour color
grep -E '^-?[0-9]+(\.[0-9]+)?$' f            # integer or decimal, optional minus
grep -E 'a{2,}' f                            # aa, aaa, ...
```

**Greedy matching.** `*` and `+` grab as much as they can while the whole pattern still matches. On `say "hi" and "bye"`, `s/".*"//` removes from the first quote to the **last** one and leaves `say `. To stop at the next quote use a negated set instead of `.`:

```bash
echo 'say "hi" and "bye"' | sed -E 's/".*"//'        # say
echo 'say "hi" and "bye"' | sed -E 's/"[^"]*"//'     # say  and "bye"
```

The same idea gives "up to the next comma" `[^,]*`, "up to the next slash" `[^/]*`, "the part before @" `[^@]+`. It is the single most useful regex habit.

**Alternation `|` and grouping `( )`.**

```bash
grep -wE 'BTC|Doge|EOS|ETH|XPM' f.crypto      # any of the words
grep -E '^(ERROR|WARN):' log                  # group, so ^ applies to both
grep -E '^ERROR|WARN:' log                    # different: "starts with ERROR" OR "contains WARN:" anywhere
grep -E '(ab)+' f                             # ab, abab, ...
grep -E 'lab(3|31|4)\.c' f
```

**Groups and back-references.** `\1` to `\9` stand for what group N matched. In sed they rebuild the replacement, in grep they demand a repeat.

```bash
echo '21-11-2024' | sed -E 's/([0-9]{2})-([0-9]{2})-([0-9]{4})/\3-\2-\1/'   # 2024-11-21
echo 'Ime Prezime' | sed -E 's/([^ ]+) ([^ ]+)/\2 \1/'                       # Prezime Ime
echo 'key=value'   | sed -E 's/^([^=]+)=(.*)$/\2=\1/'                        # value=key
grep -E '\b([a-z]+) \1\b' f                   # a word repeated twice in a row ("the the")
echo 'a1b22' | sed -E 's/[0-9]+/[&]/g'        # a[1]b[22]   & = the whole match
```

awk's `sub`/`gsub` know `&` but not `\1`; use `match()` + `substr()` or `split()` there (10.7).

**Word boundaries** (GNU grep and sed only).

```bash
grep -E '\bETH\b' f       # ETH, not ETHX or METH   (same as grep -w ETH)
grep -E '\<lab' f         # words starting with lab
grep -E 'ing\>' f         # words ending with ing
awk '$0 ~ /(^|[^[:alnum:]])ETH([^[:alnum:]]|$)/' f    # the awk way, since mawk has no \b
awk '$1 == "ETH"' f                                    # or compare a whole field
```

**Case-insensitive.**

```bash
grep -iE 'error' log
sed -E 's/error/E/Ig' log               # I flag on s
awk 'tolower($0) ~ /error/' log
```

**A variable inside the pattern.** Only the variable needs double quotes.

```bash
grep -E "^$zbor" f                       # the whole pattern is simple: double quotes are fine
grep -E '^'"$zbor"':[0-9]+$' f           # literal parts single-quoted, variable double-quoted, no spaces between
grep -F -- "$zbor" f                     # the variable may contain . or *: -F takes it literally
sed -E "s/$star/$nov/g" f                # breaks if $star contains /  ->  use another delimiter:
sed -E "s#$star#$nov#g" f
awk -v w="$zbor" '$0 ~ w' f              # pass it in with -v; w is used as a regex
awk -v w="$zbor" 'index($0, w)' f        # literal substring instead
```

**Bash's own regex test `[[ =~ ]]`.** The pattern is unquoted; quoting makes it literal. Captures land in `BASH_REMATCH`.

```bash
[[ "$n" =~ ^[0-9]+$ ]]               || { echo "Ne e cel broj"; exit 1; }
[[ "$x" =~ ^-?[0-9]+(\.[0-9]+)?$ ]]  || { echo "Ne e broj"; exit 1; }
[[ "$f" =~ \.(py|py3)$ ]]            && echo "python skripta"
if [[ "$ip" =~ ^([0-9]{1,3})\.([0-9]{1,3})\.([0-9]{1,3})\.([0-9]{1,3})$ ]]; then
    echo "prv oktet: ${BASH_REMATCH[1]}"
fi
re='^[a-z]+ [0-9]+$'; [[ "$s" =~ $re ]]    # a pattern with spaces goes in a variable
```

### 10.7 How each tool uses a regex

**grep** selects lines. With `-o` it becomes an extractor.

```bash
grep -E 'pat' f             # lines containing a match
grep -vE 'pat' f            # lines without one
grep -cE 'pat' f            # how many lines match
grep -oE 'pat' f | wc -l    # how many matches (one line can hold several)
grep -oE 'pat' f | sort -u  # the distinct things that matched
grep -lE 'pat' *.txt        # which files
grep -xE 'pat' f            # the whole line must match (= ^pat$)
```

**sed** uses a regex in two places: as an **address** (which lines) and inside **`s`** (what to change).

```bash
sed -n -E '/^ERROR/p' log                  # print matching lines (= grep)
sed -E '/^#/d; /^$/d' f                    # delete comments and blank lines
sed -E '/^BEGIN/,/^END/d' f                # delete from a matching line to another
sed -E 's/[[:space:]]+$//' f               # strip trailing whitespace
sed -E 's/^[[:space:]]+//' f               # strip leading
sed -E 's/[[:space:]]+/ /g' f              # squeeze whitespace runs to one space
sed -E 's/.*: //' f                        # keep what follows the LAST ": " (greedy)
sed -E 's/^([^:]*): .*/\1/' f              # keep what is BEFORE the first ": "
sed -n -E 's/^PORT=(.*)/\1/p' app.conf     # extract a value: print only when the s matched
sed -E '/^PORT=/ s/=.*/=8080/' app.conf    # change the value only on that line
sed -E 's/\r$//' f                         # CRLF -> LF
```

**awk** tests the whole line or one field, and has regex functions.

```bash
awk '/ERROR/' log                          # lines matching (default action is print)
awk '!/^#/' f                              # lines NOT matching
awk '$2 ~ /^[0-9]+$/' f                    # field 2 is a number
awk '$1 !~ /^lab/' f                       # field 1 does not start with lab
awk '/^10:/,/^11:/' log                    # range, like sed
awk -F'[,;]' '{print $2}' f                # regex as field separator: comma or semicolon
awk -F' +' '{print $3}' f                  # one or more spaces (the default FS already does this)
awk '{gsub(/[^0-9]/, "", $2); print $2}' f                              # keep only digits in field 2
awk 'match($0, /[0-9]+\.[0-9]+/) {print substr($0, RSTART, RLENGTH)}' f  # extract the first decimal
awk '{split($0, a, /[-\/]/); print a[3] "-" a[2] "-" a[1]}' dates.txt    # split on - or /
```

### 10.8 Worked examples from the course

**Dates `dd-mm-yyyy`** (2019/20 exam: count dated lines in a generated file)

```bash
grep -cE '^[0-9]{2}-[0-9]{2}-[0-9]{4}' out.txt                        # lines that start with a date
grep -oE '\b[0-9]{2}-[0-9]{2}-[0-9]{4}\b' out.txt                      # every date in the file
sed -E 's/\b([0-9]{2})-([0-9]{2})-([0-9]{4})\b/\3-\2-\1/g' out.txt     # to yyyy-mm-dd
grep -oE '\b[0-9]{2}-[0-9]{2}-[0-9]{4}\b' out.txt | cut -d- -f2 | sort | uniq -c   # how many per month
grep -E '^[0-9]{2}-(0[1-9]|1[0-2])-[0-9]{4}' out.txt                   # month must be 01-12
```

**IPv4 addresses** (20.11.2023 exam: a thread per IP in a file)

```bash
grep -cE '^([0-9]{1,3}\.){3}[0-9]{1,3}$' ips.txt                        # IP-shaped lines
grep -oE '\b([0-9]{1,3}\.){3}[0-9]{1,3}\b' access.log | sort | uniq -c | sort -rn   # top IPs in a log
awk -F. 'NF==4 && $1<=255 && $2<=255 && $3<=255 && $4<=255' ips.txt     # shape is not enough: 999.1.1.1 passes the regex
grep -oE '^[0-9]{1,3}' ips.txt | sort -n | uniq -c                      # count by first octet
```

**E-mails** (Lab 1 2026, Doma Z3)

```bash
grep -oE '[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}' f       # extract them
grep -oE '@[A-Za-z0-9.-]+' f | sort | uniq -c                      # count per domain
grep -vE '[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}' f       # lines WITHOUT a valid address
sed -E 's/\b[A-Za-z0-9._%+-]+@(([A-Za-z0-9-]+\.)+[A-Za-z]{2,})\b/?@\1/g' f   # censor; section 12 has the strict version
```

**Crypto values** (21.11.2024 exam: files with BTC/Doge/EOS/ETH/XPM and numbers to sum)

```bash
grep -lwE 'BTC|Doge|EOS|ETH|XPM' *.crypto                        # which files mention a coin
grep -oE '[0-9]+(\.[0-9]+)?' all.crypto | wc -l                  # how many numbers
grep -oE '[0-9]+(\.[0-9]+)?' all.crypto | awk '{s+=$1} END {print s}'   # their sum
awk '$0 ~ /(^|[^[:alnum:]])ETH([^[:alnum:]]|$)/ {print FILENAME ": " $0}' *.crypto   # whole-word ETH, in awk
```

**Key words in source files** (Lab 1 2026, Z3)

```bash
grep -l password *.py                                            # substring: also matches my_password
grep -lw password *.py                                           # whole word only
grep -nE 'password[[:space:]]*=[[:space:]]*"[^"]*"' *.py         # assignments with a quoted value
grep -ohE 'password[[:space:]]*=[[:space:]]*"[^"]*"' *.py | sed -E 's/.*"([^"]*)"/\1/'   # just the values
```

**Config files and logs**

```bash
sed -n -E 's/^[[:space:]]*port[[:space:]]*=[[:space:]]*//Ip' app.conf      # value of port=, any case and spacing
grep -oE '^\[[^]]+\]' app.ini | tr -d '[]'                                 # section names from [name] headers
awk '/^(ERROR|WARN)/ {c[$1]++} END {for (k in c) print k, c[k]}' app.log   # count per level
sed -n -E '/^2024-11-21 10:/,/^2024-11-21 11:/p' app.log                   # one hour of a log
```

**File names**

```bash
ls -1 | grep -E '^lab[0-9]+(_[0-9]+)?\.c$'         # lab4.c lab4_2.c, not lab.c
ls -1 | grep -iE '\.(jpe?g|png)$'                  # images, any case
for f in lab*.c; do mv -n "$f" "$(echo "$f" | sed -E 's/^lab([0-9]+)/Lab\1/')"; done   # rename by regex
find . -regextype posix-extended -regex '.*/Z[0-9]+\.(sh|py3?)'       # find by regex on the whole path
```

### 10.9 Debugging a regex

- Build it up on sample text: `echo 'tekst' | grep -oE 'pattern'`. `-o` shows exactly what matched, `--color` highlights it in the line.
- No match at all? Did you forget `-E` (then `+ ? ( ) |` are literal)? Is the pattern in single quotes? Is there a hidden `\r` at the line ends? `cat -A f` shows tabs as `^I`, carriage returns as `^M` and line ends as `$`.
- Too much matched? A `.*` is being greedy. Replace `.` with a negated set: `[^"]`, `[^,]`, `[^ ]`.
- `\d`, `\w`, `\s` do nothing useful outside `grep -P`. Use `[0-9]`, `[[:alnum:]_]`, `[[:space:]]`.
- In awk (mawk) `\b \< \> \w \s` are not supported. Use POSIX classes or compare whole fields.
- Put `echo` in front of the whole command to see what bash hands to the program (10.1).

### 10.10 Regex quick table

| Pattern | Meaning | Example |
|---|---|---|
| `abc` | literal text, anywhere in the line | `abc` in `xabcx` |
| `.` | any one character | `l.b` matches `lab`, `l5b` |
| `[abc]` `[a-z]` `[^0-9]` | set, range, negated set | |
| `[[:digit:]]` `[[:alpha:]]` `[[:space:]]` `[[:alnum:]]` | POSIX classes, inside brackets | |
| `^` `$` | line start, line end | `^#`, `\.c$` |
| `*` `+` `?` | 0 or more, 1 or more, 0 or 1 of the previous item | `[0-9]+` |
| `{n}` `{n,}` `{n,m}` | exact, at least, between | `[0-9]{1,3}` |
| `a\|b` | either | `BTC\|ETH` |
| `( )` | group: for alternation, quantifiers and `\1` | `([0-9]{1,3}\.){3}` |
| `\1` | what group 1 matched (sed replacement, grep pattern) | `s/(.)(.)/\2\1/` |
| `&` | the whole match (sed replacement, awk sub/gsub) | `s/[0-9]+/<&>/` |
| `\b` `\<` `\>` | word boundary, word start, word end (GNU grep and sed only) | `\bETH\b` |
| `\.` `\$` `\*` `\[` | the literal character | |

---

## 11. `grep` in depth

`grep [options] PATTERN [files]`. Prints the **lines** that match. Exit status: 0 found, 1 nothing found, 2 error, so it works directly in `if`.

| Option | Meaning |
|---|---|
| `-E` | extended regex (`+ ? \| ( ) { }` work without backslashes); `egrep` is the same |
| `-F` | fixed string, no regex (fast, safe for dots) |
| `-i` | ignore case |
| `-v` | invert: lines that do **not** match |
| `-w` | whole words only (`BTC` but not `BTCX`) |
| `-x` | whole line must match |
| `-n` | show line numbers |
| `-c` | count matching lines (per file) |
| `-l` / `-L` | only names of files with / without a match |
| `-o` | print only the matched part, one per line |
| `-q` | quiet: no output, just the exit status (for `if`) |
| `-r` | recursive through directories (`--include='*.py'` to filter) |
| `-H` / `-h` | always / never prefix the file name |
| `-A 2` / `-B 2` / `-C 2` | lines of context after / before / both |
| `-m 1` | stop after the first match |
| `-s` | no "file not found" errors |

The pattern is a regex: section 10 explains the syntax, the shell-versus-regex symbol clash and the `-E` dialect used here.

Recipes:

```bash
grep -E '^[0-9]{2}-[0-9]{2}-[0-9]{4}' out.txt | wc -l        # lines starting with dd-mm-yyyy  (2019/20)
grep -c -E '^[0-9]{2}-[0-9]{2}-[0-9]{4}' out.txt             # same count, no pipe
grep -wE 'BTC|Doge|EOS|ETH|XPM' "$f"                         # any of the words, whole word  (21.11.2024)
grep -l -wE 'BTC|Doge|EOS|ETH|XPM' *.crypto                  # which files have them
grep -q password "$f" && cp "$f" .                           # test, then act             (Lab 1 2026 Z3)
grep -Hn password ./*.py                                     # file:line:text
grep -oE '([0-9]{1,3}\.){3}[0-9]{1,3}' log | sort -u         # extract all IPv4 addresses
grep -oE '[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}' contacts.txt   # extract e-mails
grep -v '^#' s.sh | grep -v '^$'                             # drop comments and blank lines
grep -rn 'TODO' --include='*.c' .
ls -1 | grep -E '\.c$'                                       # .c files via grep            (Lab 1 2026 Doma Z2)
```

`grep -c` counts **lines**; to count **occurrences** use `grep -o word f | wc -l`.

---

## 12. `sed` in depth

Stream editor: reads line by line, applies commands, prints the result. Does not change the file unless `-i`.

```bash
sed 's/old/new/' f          # first occurrence per line
sed 's/old/new/g' f         # every occurrence
sed -E 's/(a|b)+/X/' f      # extended regex, groups without backslashes
sed -n '5,10p' f            # -n: print nothing by default; p: print these lines
sed -i 's/\r$//' f          # edit the file in place; -i.bak keeps a backup
sed -e 's/a/b/' -e 's/c/d/' f   # several commands (or separate with ;)
```

| Address | Lines affected |
|---|---|
| none | all |
| `5` | line 5 |
| `5,10` | 5 to 10 |
| `$` | last line |
| `/re/` | lines matching re |
| `/start/,/end/` | from a match to the next match |
| `1~2` | every 2nd line starting at 1 (GNU) |
| `addr!` | all lines **except** the address |

| Command | Does |
|---|---|
| `s/re/rep/flags` | substitute; flags: `g` all, `2` only the 2nd, `i` ignore case, `p` print, `w f` write |
| `d` | delete the line |
| `p` | print (with `-n`) |
| `a\text`, `i\text`, `c\text` | append after, insert before, change the line |
| `y/abc/ABC/` | transliterate characters |
| `=` | print the line number |
| `q` | quit |

Inside the replacement: `&` is the whole match, `\1`…`\9` are the groups. Any character can be the delimiter: `s#/usr/bin#/opt#` avoids escaping slashes.

Recipes:

```bash
sed -n '2,4p' f                        # print lines 2-4
sed '/^$/d' f                          # delete empty lines
sed '/^#/d' f                          # delete comment lines
sed 's/^[ \t]*//' f                    # strip leading whitespace
sed -E 's/.*: //' f                    # keep only what follows ": "
sed -E 's/([0-9]+)-([0-9]+)-([0-9]+)/\3.\2.\1/' f    # 20-11-2024 -> 2024.11.20
sed -i "s/$old/$new/g" f               # with variables: double quotes
sed 's/\r$//' f                        # CRLF -> LF
echo "lab5.tar.gz" | sed 's/\..*//'    # lab5
```

**Censor valid e-mails (Lab 1 2026, Doma Z3)**, local part replaced with `?`:

```bash
sed -E 's/\b[A-Za-z0-9._%+-]+@(([A-Za-z0-9-]+\.)+[A-Za-z]{2,})\b/?@\1/g' vezba12.txt   # simple version
```

Reading it: `\b` word boundary, `[A-Za-z0-9._%+-]+` the part before `@`, then the whole domain in group 1: `([A-Za-z0-9-]+\.)+` one or more labels each ending with a dot, `[A-Za-z]{2,}` the top-level domain. The replacement writes `?@` plus group 1. An address like `contact@feit` has no dot in the domain and is left alone, which is the "valid only" requirement. The simple version still censors two invalid forms: `deans..office@feit.ukim.edu.mk` (dots are allowed anywhere in the local part, so `..` passes) and `user@feit.ukim.edu.` (the final dot is just a word boundary after `edu`). The full solution in `course/2026/Labs/Lab1/Doma_Z3.sh` fixes both by matching the local part as dot-separated segments and by requiring a non-domain character or line end after the top-level domain.

---

## 13. `awk` essentials

A small language for columns. Each line is a record, split into fields `$1 $2 …`; `$0` is the whole line, `NF` the number of fields, `NR` the line number. Default separator is any run of spaces/tabs, so it beats `cut` on aligned output.

```bash
awk '{print $1}' f                     # first column
awk '{print $NF}' f                    # last column
awk -F: '{print $1, $3}' /etc/passwd   # custom separator, output separated by space
awk -F: -v OFS='\t' '{print $1, $3}' f # output separator tab
awk '$3 > 100' f                       # lines where column 3 > 100 (no action = print)
awk '/ERROR/ {n++} END {print n+0}' f  # count matching lines
awk '{s += $2} END {print s}' f        # sum column 2
awk '{s += $2} END {printf "%.2f\n", s/NR}' f        # average
awk -v a="$found" -v b="$total" 'BEGIN {printf "%.1f%%\n", a*100/b}'   # decimal percentage
awk 'NR==2' f                          # print line 2
awk 'length($0) > 80' f                # long lines
awk '{c[$1]++} END {for (k in c) print c[k], k}' f | sort -rn        # frequency of column 1
awk '{gsub(/,/, "."); print}' f        # replace in every line (regex), then print
awk 'BEGIN{FS=","} {print toupper($2)}' data.csv
```

Structure: `BEGIN { before any line }  pattern { per matching line }  END { after the last line }`. Strings in awk use double quotes; keep the whole program in single quotes and pass shell values with `-v`.

---

## 14. Text toolbox

| Command | Flags | Example |
|---|---|---|
| `cut` | `-d: -f1,3` fields by delimiter, `-c1-5` characters | `cut -d, -f2 data.csv` (single-char delimiter only) |
| `sort` | `-n` numeric, `-r` reverse, `-k2` by column 2, `-t:` delimiter, `-u` unique, `-h` human sizes | `sort -t: -k3 -n /etc/passwd` |
| `uniq` | `-c` count, `-d` only duplicates, `-u` only unique; **input must be sorted** | `sort f \| uniq -c \| sort -rn` |
| `tr` | translate chars, `-d` delete, `-s` squeeze repeats | `tr a-z A-Z`, `tr -d '\r'`, `tr -s ' '`, `tr ' ' '\n'` |
| `head` / `tail` | `-n N`, `tail -n +N` from line N, `tail -f` | `tail -n +2 f` skips the header |
| `wc` | `-l -w -c -m` | `wc -l < f` (no file name in output) |
| `paste` | join files line by line, `-d,` | `paste -d, a b` |
| `rev` | reverse each line | `rev f` |
| `nl` | number lines | |
| `column` | `-t` align into a table | `column -t -s,` |
| `tee` | `-a` append | `cmd \| tee log.txt` |
| `diff` / `cmp` | `-u` unified, `-q` quiet | `diff -u a b`, `cmp -s a b && echo isti` |
| `date` | `+FORMAT`, `-d` parse | `date +%Y-%m-%d`, `date +%s` (epoch seconds), `date -d '2024-11-21' +%A` |
| `seq` | `seq 1 10`, `seq 0 5 100` (step), `-w` pad | |
| `printf` | C-style format, repeats the format for extra args | `printf "%-10s %5d\n" "$name" "$n"`, `printf '%s\n' "${arr[@]}"` |
| `bc` | calculator, `scale=N` decimals, `-l` math lib | `echo "scale=2; 7/3" \| bc` |
| `expr` | old arithmetic and string ops | `expr length "$s"`, `expr "$s" : '.*'` |
| `basename` / `dirname` | | `basename "$f" .c` |
| `md5sum` / `sha256sum` | checksums | `sha256sum f` |
| `split` | split a file into pieces | `split -l 1000 big part_` |
| `dos2unix` | fix line endings | |

`echo` vs `printf`: `echo -e` interprets `\n`, `echo -n` no newline, but behaviour varies between shells; `printf` is predictable and does formatting. Use `printf` for anything beyond a plain message.

---

## 15. Processes, jobs, signals, permissions, system

### Jobs and background

| | |
|---|---|
| `cmd &` | run in background, prints the job number and PID |
| `$!` | PID of the last background command |
| `jobs` | list jobs of this shell |
| `fg %1` / `bg %1` | bring to foreground / continue in background |
| `Ctrl+Z` / `Ctrl+C` | suspend / interrupt the foreground job |
| `wait` / `wait $pid` | block until all / one background job finishes; `$?` is its status |
| `nohup cmd &` | keep running after logout |
| `sleep 3` | pause 3 seconds (`sleep 0.5` works on Linux) |
| `time cmd` | how long it took |
| `timeout 5 cmd` | kill cmd after 5 seconds |

### Inspecting and killing processes

| | |
|---|---|
| `ps aux` | all processes (BSD style); `ps -ef` (System V style) |
| `ps -p PID -o pid,ppid,stat,cmd` | specific process, chosen columns |
| `pgrep -x firefox` | PIDs whose **name** is exactly firefox; `-f` matches the full command line; `-l` shows names |
| `pidof firefox` | same idea, older |
| `pkill -x firefox` | kill by name |
| `kill PID` | send SIGTERM (15, polite stop) |
| `kill -9 PID` | SIGKILL, cannot be ignored |
| `kill -STOP PID` / `kill -CONT PID` | pause / resume |
| `kill -l` | list signal names |
| `kill %1` | kill job 1 |
| `killall name` | kill all by name |
| `top` / `htop` | live view; `q` quits |
| `trap 'rm -f "$tmp"' EXIT` | run cleanup when the script exits (also on `INT`, `TERM`) |
| `exec cmd` | replace the shell with cmd (no return) |

```bash
firefox &                      # Lab 1 2026, Doma Z1
pid=$!                          # or: pid=$(pgrep -x firefox | head -1)
echo "PID: $pid"
kill "$pid"
sleep 1
ps -p "$pid" > /dev/null && echo "seuste raboti" || echo "ubien"
```

Timing a program from a script: `start=$(date +%s); ./prog; echo "$(( $(date +%s) - start )) s"`, or the built-in `$SECONDS` counter.

### Permissions

`ls -l` shows `-rwxr-xr-- 1 david david 1234 Nov 2 10:00 s.sh`: type, then three triples (owner, group, others) of read/write/execute.

| | |
|---|---|
| `chmod +x s.sh` | executable for everyone; `u+x` owner only; `go-w` remove write from group and others |
| `chmod 755 s.sh` | octal: r=4, w=2, x=1 per triple; 755 = rwx r-x r-x; 644 = rw- r-- r-- |
| `chmod -R` | recursive |
| `chown user:group f` | change owner |
| `umask` | default permission mask for new files (022 → files 644, dirs 755) |
| `id`, `whoami`, `groups` | who am I |
| `sudo cmd` | run as root |

### System and help

| | |
|---|---|
| `man cmd` | manual (`/pattern` to search, `q` to quit); `man -k word` or `apropos word` to find commands |
| `cmd --help` | short usage |
| `uname -a` | kernel and machine |
| `df -h`, `du -sh dir`, `free -h` | disk, directory size, memory |
| `env`, `echo "$PATH"`, `echo "$HOME"` | environment |
| `history`, `!!`, `!grep` | command history, repeat last, repeat last starting with grep |
| `alias ll='ls -la'` | shortcuts (put in `~/.bashrc`) |
| `uptime`, `who`, `hostname` | |

---

## 16. Exam recipes

Patterns that appear again and again in the first partial. Each is a complete skeleton.

**R1. Argument check with usage**

```bash
if [ "$#" -lt 2 ]; then
    echo "Upotreba: $0 <zbor> <datoteka> [...]"
    exit 1
fi
```

**R2. Default when no argument is given**

```bash
name="${1:-naredbi.txt}"
```

**R3. File exists in the current dir and also somewhere below: delete the local copy** (Lab 1 2026 Z2, 19.11.2019)

```bash
sub=$(find . -mindepth 2 -type f -name "$name" | head -1)
if [ -f "./$name" ] && [ -n "$sub" ]; then
    rm "./$name"
    cat "$sub"
elif [ -f "./$name" ]; then
    cat "./$name"
elif [ -n "$sub" ]; then
    cat "$sub"
else
    echo "Datotekata ne e pronajdena"; exit 1
fi
```

**R4. Percentage of arguments that exist** (Lab 1 2026 Z1): see the end of section 8.

**R5. Find files by extension, keep those containing a word, copy them, print the lines** (Lab 1 2026 Z3)

```bash
copied=()
while IFS= read -r -d '' f; do
    if grep -q 'password' "$f" && [ "$(dirname "$f")" != "." ]; then
        cp "$f" . && copied+=("$(basename "$f")")
    fi
done < <(find . -type f \( -name '*.py' -o -name '*.py3' \) -print0)
for b in "${copied[@]}"; do grep -Hn 'password' "./$b"; done
```

**R6. Merge matching files into one and hand it to a Python script** (21.11.2024)

```bash
: > all.crypto                                   # create/empty the output file
while IFS= read -r -d '' f; do
    grep -qwE 'BTC|Doge|EOS|ETH|XPM' "$f" && cat "$f" >> all.crypto
done < <(find . -type f -name '*.crypto' -print0)
n=$(./Z2.py3 all.crypto)                         # the Python script prints the count
echo "Pronajdeni kripto-vrednosti: $n"
```

**R7. Count lines matching a pattern in a generated file** (2019/20)

```bash
out="${f}_out"
datumi=$(grep -cE '^[0-9]{2}-[0-9]{2}-[0-9]{4}' "$out")
```

**R8. Work inside every directory with a given extension** (30.01.2019)

```bash
for d in *.ecf/; do
    [ -d "$d" ] || continue
    ( cd "$d" && ls -1 | wc -l )                 # subshell: cd does not leak out
done
```

**R9. Run a command per argument and report success**

```bash
for cmd in "$@"; do
    if "$cmd" > /dev/null 2>&1; then echo "$cmd: OK"; else echo "$cmd: greska ($?)"; fi
done
```

**R10. Decide by extension**

```bash
case "$1" in
    *.py3) python3 "$1" ;;
    *.sh)  bash "$1" ;;
    *)     echo "Nepoznat tip"; exit 1 ;;
esac
```

**R11. Temporary files with cleanup**

```bash
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
```

**R12. Read a file of commands and run each with timing** (19.11.2019 style)

```bash
while IFS= read -r cmd; do
    [ -z "$cmd" ] && continue
    start=$(date +%s%N)
    bash -c "$cmd" > /dev/null 2>&1
    echo "$cmd: $(( ($(date +%s%N) - start) / 1000000 )) ms"
done < naredbi.txt
```

---

## 17. Pitfalls checklist

- `x = 5` is a command named `x` with two arguments. No spaces around `=` in assignments.
- `[$a = $b]` or `[ $a=$b ]`: `[` needs spaces on both sides of every token.
- Unquoted variables: `rm $f` with `f="my file"` deletes two wrong things. Quote.
- Empty variable in `[ ]`: `[ $x = a ]` becomes `[ = a ]`. Quote: `[ "$x" = a ]`.
- `rm -rf "$dir/"` with an empty `dir` is `rm -rf /`. Use `"${dir:?}/"` or test it first.
- `==` and `<` in `[ ]` compare strings. Numbers use `-eq -lt …` or `(( ))`.
- `$(( ))` is integer only: `7/2` is `3`. Use `bc` or `awk` for decimals.
- `cmd | while read …; do count=…; done` loses `count`. Use `done < <(cmd)` or `done < file`.
- `for f in $(ls)` breaks on spaces. Use `for f in *` or `find -print0`.
- `find . -name *.c` unquoted: bash expands `*.c` first. Quote the pattern.
- `cd somewhere` can fail and the script continues in the wrong directory: `cd x || exit 1`.
- `sort f > f` empties the file before sort reads it.
- `echo $(cmd)` flattens newlines; `echo "$(cmd)"` keeps them.
- `grep -c` counts lines, not occurrences.
- `uniq` only merges **adjacent** duplicates: sort first.
- CRLF files: `$'\r': command not found`. `dos2unix`.
- `#!/bin/sh` is not bash: no arrays, no `[[ ]]`, no `$'…'`. Use `#!/bin/bash`.
- A background job's exit status is only available via `wait $pid`.
- Globs that match nothing stay literal: `for f in *.txt` runs once with `f='*.txt'`. Guard with `[ -e "$f" ]`.
- A regex outside single quotes: bash eats it first. `[0-9]{1,3}` becomes `[0-9]1 [0-9]3` (brace expansion), `$` starts a variable, `*` may expand to file names.
- `grep` or `sed` without `-E`: `+ ? ( ) | { }` are literal characters, so the pattern quietly matches the wrong thing or nothing.
- `\d` is not a digit in grep, sed or awk. `[0-9]` is.
- `${f%.*}`, `case`, `[[ == ]]` and `find -name` use glob syntax. Only `[[ =~ ]]`, grep, sed, awk and `find -regex` use regex.
- `.*` is greedy and runs to the last possible match. `[^"]*`, `[^,]*` stop at the next quote or comma.

---

## 18. Quick reference

| Command | What it does | Typical call |
|---|---|---|
| `ls` | list files | `ls -la`, `ls -1` |
| `cd`, `pwd` | change / print directory | `cd "$d" \|\| exit 1` |
| `mkdir`, `rmdir` | make / remove directory | `mkdir -p a/b` |
| `touch` | create empty file | |
| `cp`, `mv`, `rm` | copy, move, delete | `cp -r src dst`, `rm -rf "$tmp"` |
| `cat`, `less`, `head`, `tail` | show content | `head -n 3`, `tail -f log` |
| `wc` | count lines/words/bytes | `wc -l < f` |
| `find` | search files by name/type/size/time | `find . -type f -name '*.c'` |
| `grep` | lines matching a regex | `grep -rnE 'a\|b' .`, `grep -q`, `grep -l` |
| `sed` | edit a stream | `sed -E 's/x/y/g'`, `sed -n '1,3p'`, `sed -i` |
| `awk` | column processing | `awk -F: '{print $1}'`, `awk '{s+=$2} END{print s}'` |
| `cut` | columns by delimiter | `cut -d, -f2` |
| `sort`, `uniq` | sort, dedupe/count | `sort \| uniq -c \| sort -rn` |
| `tr` | translate/delete characters | `tr a-z A-Z`, `tr -d '\r'` |
| `xargs` | stdin lines to arguments | `find … -print0 \| xargs -0 rm` |
| `tee` | write to file and stdout | `cmd \| tee out.txt` |
| `diff`, `cmp` | compare | `diff -u a b` |
| `date`, `seq`, `bc`, `printf` | time, sequences, math, formatting | `date +%s`, `seq 1 10`, `printf '%5.2f\n' 1.5` |
| `basename`, `dirname` | path parts | `basename "$f" .c` |
| `chmod`, `chown` | permissions, owner | `chmod +x s.sh`, `chmod 644 f` |
| `ps`, `pgrep`, `pidof` | list / find processes | `ps aux`, `pgrep -x name` |
| `kill`, `pkill`, `killall` | send signals | `kill -9 "$pid"` |
| `jobs`, `fg`, `bg`, `wait` | job control | `cmd & wait $!` |
| `sleep`, `time`, `timeout` | pause, measure, limit | `timeout 5 ./prog` |
| `trap` | run on signal/exit | `trap 'cleanup' EXIT` |
| `read` | input into variables | `read -r -p "Ime: " name` |
| `test`, `[`, `[[` | conditions | `[ -f "$f" ]`, `[[ $f == *.py ]]` |
| `echo`, `printf` | output | `printf '%s\n' "${arr[@]}"` |
| `export`, `env`, `unset` | environment | `export PATH="$PATH:~/bin"` |
| `source`, `.` | run in the current shell | `source ~/.bashrc` |
| `man`, `--help`, `type` | help | `man find`, `type ls` |
| `du`, `df`, `free`, `uname` | sizes, disk, memory, system | `du -sh *` |
| `dos2unix` | CRLF to LF | |
| `shellcheck` | lint a script | `shellcheck s.sh` |
