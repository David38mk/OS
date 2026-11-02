#!/bin/bash

# Задача 3: Да се искористи датотеката vezba12.txt која содржи контакт информации за ФЕИТ. Да се искористи наредбата
# SED за да се најдат сите валидни е-маил адреси и да се цензурираат податоците од е-маил адресите пред знакот @.
# Пример е зададен подолу. Да се внимава дека треба да се најдат валидните е-маил адреси.
#
# Example input:
#   Official contact email: contact@feit.ukim.edu.mk
# Expected output:
#   Official contact email: ?@feit.ukim.edu.mk

file="${1:-vezba12.txt}"

if [ ! -f "$file" ]; then
    echo "Datotekata ne e pronajdena"
    exit 1
fi

# prefiks (1) | lokalen del (2): delovi razdeleni so edna tocka, bez tocka na pocetok/kraj i bez ".."
# | @ | domen (4): labeli so tocki, TLD >= 2 bukvi | sufiks (6)
# petlja (:a ... ta) za da se opfatat i dve adresi razdeleni so eden prazen znak
sed -E ':a; s/(^|[^A-Za-z0-9._%+@-])([A-Za-z0-9%+_-]+(\.[A-Za-z0-9%+_-]+)*)@(([A-Za-z0-9-]+\.)+[A-Za-z]{2,})([^A-Za-z0-9.-]|$)/\1?@\4\6/; ta' "$file"
