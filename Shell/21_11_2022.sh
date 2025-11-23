#!/bin/bash

if [ "$#" -lt 3 ]; then
    echo "Nedovolno argumenti vo komandna linija"
    exit 1
else
    if [[ "$1" == *.py3 ]]; then
        python_scripta=$1
    else
        python_scripta="prva.py3"
    fi
fi

zbor=$2
prebaruvanje=$(find . -type f -name "$python_scripta")

if [ -n "$prebaruvanje" ]; then
    shift
    shift
    vkupno=0

    for dat in "$@"; do
        z=$(./"$python_scripta" f "$dat")
        vkupno=$((vkupno + z))
        echo "${dat}: ${z}"
    done

    echo "Vkupno: ${vkupno}"
else
    echo "Ne e pronajden skriptot: $python_scripta"
fi

#  Да се напише SHELL скрипта која како аргумент добива име на Python скрипта, збор за
# пребарување и листа од имиња на датотеки. Доколку не добие како прв аргумент име на Python
# скрипта (аргумент што завршува на .py3) тогаш зема како име на скрипта „prva.py3“. Shell
# скриптата треба да провери дали има Python скрипта со такво име во тековниот директориум
# или поддиректориумите и доколку нема, завршува Shell скриптата со извршување. Доколку
# постои Python скрипта со такво име (тековниот директориум или поддиректориумите), тогаш
# Shell скриптата треба со помош на Python скриптата, да најде и отпечати на екран, колку пати ќе
# го пронајде зборот (што се праќа како втор аргумент на Shell) за сите датотеки што се праќаат
# како аргументи (од трет аргумент па натаму). На крај скриптата треба да отпечати колку
# вкупно бил пронајден зборот во сите датотеки. 