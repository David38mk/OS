#!/bin/bash

imenici=$(ls | grep '.*\.ecf')
br_imenici=$(ls | grep '.*\.ecf' | wc -l)
if [ "$br_imenici" -lt 1 ]; then
	echo "Nema imenici"
else
	prv=$(echo "$imenici" | head -1)
	imenici=$(echo "$imenici" | cut -1)
	for imenik in $imenici; do
		cat "$imenik" >> "$prv"
	done
	./python_scripta "$prv"
fi

# Да се напише Shell скрипта која што ги наоѓа сите именици во тековниот директориум т.е. ги
# бара сите датотеки кои што завршуваат на екстензија .vcf. Доколку има повеќе именици, тогаш на крајот
# во првиот именик од листата именици се додава содржината на другите именици и се повикува Python
# скриптата со името на првиот именик