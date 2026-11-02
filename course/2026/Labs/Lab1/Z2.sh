#!/bin/bash

# Задача 2: Да се напише SHELL скрипта која како аргумент од командна линија добива име на влезна датотека. Доколку не
# се прати аргумент на командна линија, тогаш се пребарува датотека со име „naredbi.txt“ во тековниот директориум и
# неговите под-директориуми. Доколку датотеката ја има во тековниот директориум и дополнително во некој од
# поддиректориумите, тогаш да се избрише од тековниот директориум. На крај, да се отпечати содржината на датотеката
# (доколку истата е пронајдена).

name="${1:-naredbi.txt}"

if [ -z "$(find . -type f -name "$name" | head -1)" ]; then
    echo "Datotekata ne e pronajdena"
    exit 1
fi

sub="$(find . -mindepth 2 -type f -name "$name" | head -1)"

if [ -f "./$name" ] && [ -n "$sub" ]; then
    rm "./$name"
    echo "Datotekata e izbrisana od tekovniot direktorium"
    cat "$sub"
elif [ -f "./$name" ]; then
    cat "./$name"
else
    cat "$sub"
fi
