#!/bin/bash

z=( $(find . -type f -name '*.ip') )

if [ ${#z[@]} -gt 0 ]; then
    for file in "${z[@]}"; do
        
        out="${file}_out.txt"

        # Create empty file
        touch "$out"

        # Run python script
        ./python_sript "$file" o "$out"

        # Read file line by line using `for`
        for IP in $(cat "$out"); do
            ping -c 1 "$IP" > /dev/null 2>&1
            komand=$?
            
            if [ $komand -eq 0 ]; then
                echo "$IP OK"
            else
                echo "$IP NOT OK"
            fi
        done

    done
fi

# Да се напише SHELL скрипта која треба да ги најде сите датотеки (во тековниот директориум и
# поддиректориумите) кои имаат наставка (екстензија) „.ip“. Потоа, користејќи ја Python
# скриптата од првата задача, да се генерираат сите можни IP адреси од секоја пронајдена
# датотека одделно. Откако ќе се изгенерираат IP адресите, за секоја IP адреса да се провери дали
# има пристап до истата, и анализата да се испише на екран (да се испише секоја IP адреса и до
# истата ОК или NOT во продолжение). Проверката дали има пристап или не до дадена IP адреса
# да се прави со наредбата „ping -c 1 $IP > /dev/null 2>&1; “