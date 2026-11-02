#!/usr/bin/python3

import os
import sys

if len(sys.argv) != 2:
	print("Nevaliden broj na arg")
	sys.exit(1)

file_name=sys.argv[1]
org = open(file_name,'r')
M_31 = (1,3,5,7,8,10,12)
M_30 = (4,6,9,11)

with open(f"{file_name}_out",'w') as file:
	for line in org.readlines():
		dates=[]
		for word in line.split():
			varr=word.split('-')
			if len(varr)!=3:
				continue
			if len(varr[3]) != 4:
				continue
			if int(varr[1]) in M_31 and int(varr[0])>0 and int(varr[0] < 32):
				dates.append(word)
				continue
			if int(varr[1]) in M_30 and int(varr[0])>0 and int(varr[0]<31):
				dates.append(word)
				continue
			if int(varr[1]) ==2 and int(varr[0])>0 and int(varr[0]) <30:
				dates.append(word)
				continue
		for date in dates:
			file.write(f"{date} ")
			for word in line.split():
				if word not in dates:
					file.write(f"{word} ")
			file.write("\n")


# Да се напише Python скрипта која како аргумент добива име на влезна датотека. Скриптата треба да
# креира излезна датотека со исто име како и влезната, само со наставка “_out”. Скриптата треба да ја
# измине влезната датотека линија по линија, и во излезната датотека да ја запише линијата, така што
# доколку во линијата текст се наоѓа датум, датумот да дојде прв во линијата, па потоа, без менување на
# редоследот, да дојде и останатиот текст од линијата. Форматот на датумот е “DD-MM-YYYY”, притоа, прво се проверува дали датумот е валиден датум
# доколку има повеќе од еден датум, се запишуваат во излезната датотека онолку линии колку што има
# различни датуми во влезната линија, притоа текстот што следи е без датуми. Доколку скриптата се
# повика со втор аргумент и доколку вториот аргумент е валиден датум, тогаш дополнително и на екран
# се печатат оние линии каде што го има тој датум. Зборовите во датотеката се одделени со празно место. 