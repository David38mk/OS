#!/bin/bash

# Задача 1. Да се започне процес за firefox програмот и да се најде неговото процесно ID. Потоа, користејќи го тоа ID
# да се убие процесот

firefox &                 # start vo pozadina
pid_bg=$!                 # PID na posledniot pozadinski proces
sleep 3                   # kratko cekanje firefox da se podigne

pid=$(pgrep -x firefox | head -1)   # PID preku ime na procesot

if [ -z "$pid" ]; then
    echo "Procesot firefox ne e pronajden"
    pid="$pid_bg"
fi

echo "PID (\$!): $pid_bg"
echo "PID (pgrep): $pid"

kill "$pid"
sleep 1

# proverka dali procesot e ubien
if ps -p "$pid" > /dev/null 2>&1; then
    echo "Procesot $pid seuste raboti"
else
    echo "Procesot $pid e ubien"
fi
