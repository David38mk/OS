#!/bin/bash

# Задача 3: Да се напише SHELL Скрипта која треба да ги најде во тековниот директориум и поддиректориумите сите
# датотеки што имаат екстензија .py или py3. Потоа, треба да ти копира во тековниот директориум само оние датотеки (од
# најдените) во чија содржина има клучен збор „password“. На крај скриптата ги печати само оние линии од датотеките
# што ги копира каде што го има клучниот збор.

copied=()

while IFS= read -r -d '' f; do
    if grep -q 'password' "$f"; then
        # datoteki vekje vo tekovniot direktorium se preskoknuvaat
        if [ "$(dirname "$f")" != "." ]; then
            cp "$f" .
            copied+=("$(basename "$f")")
        fi
    fi
done < <(find . -type f \( -name '*.py' -o -name '*.py3' \) -print0)

if [ ${#copied[@]} -eq 0 ]; then
    echo "Nema kopirani datoteki"
    exit 0
fi

for b in "${copied[@]}"; do
    grep -Hn 'password' "./$b"
done
