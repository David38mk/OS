#!/bin/bash

# Задача 1: Да се напише SHELL Скрипта која како аргументи од командна линија добива имиња на датотеки. Доколку не
# добие ниту еден аргумент од командна линија, скриптата прекинува. Скриптата треба да провери кои датотеки ги има во
# тековниот директориум (или поддиректориуми). На крај, скриптата треба да отпечати процентот од датотеки кои се
# најдени.

if [ $# -eq 0 ]; then
    echo "Nema argumenti"
    exit 1
fi

total=0
found=0

for name in "$@"; do
    total=$((total + 1))
    if [ -n "$(find . -type f -name "$name" | head -1)" ]; then
        echo "$name - najdena"
        found=$((found + 1))
    else
        echo "$name - ne e najdena"
    fi
done

echo "Procent na najdeni datoteki: $((found * 100 / total))%"
