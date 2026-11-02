#!/usr/bin/python3

import sys
import os

if len(sys.argv) < 3:
	print("Nedovolen broj na argumenti")
	sys.exit(1)
if sys.argv[2] == 'a':
	vlezna_dat=open(sys.argv[1],"r")
elif sys.argv[2] == 'o':
	if len(sys.argv) < 4:
		print("Ne vnesovte izlezna datoteka za znamenceto o")
		sys.exit(1)
	else:
		vlezna_dat=open(sys.argv[1],"r")
		izlezna_dat=open(sys.argv[3],"w")
else:
	print("Nevalidno znamence")
	sys.exit(1)

for lines in vlezna_dat.readlines():
	words=lines.split() # split() - na praznite mesta, split('-') split(' ')
	IP_adress=""
	IP_adress_mask=""
	for word in words:
		IP=word.split('.')
		if len(IP) == 4:
			mask_flag=True
			ip_flag=True
			for num in IP:
				if int(num) < 0 or int(num) > 255:
					ip=False
					break
			if ip==True:
				for i, num in enumerate(IP):
					if int(num)!=0 or int(num)!=255:
						mask_flag=False
						break
					if int(num) == 255 and i!=0 and num[i-1]!=255:
						mask_flag=False
				if mask_flag:
					IP_adress_mask=word
				else:
					IP_adress=word
	if sys.argv[1]=='a':
		ip = IP_adress.split('.')
		for i in range(int(ip[0]),256):
			for j in range(int(ip[1]),256):
				for k in range(int(ip[2]),256):
					for h in range(int(ip[3]),256):
						print(f"{i}.{j}.{k}.{h}")
					
	elif sys.argv[1]=='o':
		ip = IP_adress.split('.')
		ip_mask = IP_adress_mask.split('.')
		for i in range(int(ip[0]),int(ip_mask[0])+1):
			for j in range(int(ip[1]),int(ip_mask[1])+1):
				for k in range(int(ip[2]),int(ip_mask[2])+1):
					for h in range(int(ip[3]),int(ip_mask[3])+1):
						izlezna_dat.write(f"{i}.{j}.{k}.{h}\n")

# Да се напише Python скрипта со име „prva.py3“ која како аргумент добива влезна датотека и знаменце.
# Знаменцето може да биде „а“. Може да има и опционално дополнително знаменце „о“, после кое следи
# име на излезна датотека. Доколку нема дополнително знаменце „о“ и излезна датотека, се печати на
# екран, инаку, се печати во излезната датотека. Скриптата треба, да ја измине влезната датотека и да ги
# пронајде во секој ред, IP адресите, притоа доколку најде во некој ред IP адреса, можно е во истиот ред
# да има и маска за IP адреса. Форматот на IP адресите е х.х.х.х, каде х е позитивен цел број во опсег од 0
# до 255, додека пак маската е х:х:х:х. Скриптата треба, ако најде само IP адреса во редот, да ја отпечати,
# но доколку најде маска во редот, да ги отпечати сите можни IP адреси во опсегот од IP адресата до
# маската, секоја во нов ред. Доколку има знаменце „а“, тогаш ги игнорира маските, и ги печати сите IP
# адреси од таа IP адреса што ќе ја најде до 255.255.255.255 (да се земе како маска за сите IP адреси да е
# оваа). 
