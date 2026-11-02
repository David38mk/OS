#!/usr/bin/python3

import os
import sys

if len(sys.argv) < 2:
    print("Ne vnesovte datoteka za pregled")
    sys.exit()
else:
    CryptoValuti = {
        "BTC": [],
        "Doge": [],
        "EOS": [],
        "ETH": [],
        "XPM": []
    }

    with open(sys.argv[1], "r") as f:
        Full_string = f.read()

    words = Full_string.split()

    # Extract numbers after crypto symbols
    for i, word in enumerate(words):
        if word in CryptoValuti:
            if i + 1 < len(words):
                next_word = words[i + 1]
                # Expect format something like: X123, F245, etc.
                if len(next_word) > 1 and next_word[1:].isnumeric():
                    CryptoValuti[word].append(int(next_word[1:]))

    # Calculate max rise and max fall
    for key, niza in CryptoValuti.items():
        max_rise = 0
        max_fall = 0

        for i in range(len(niza) - 1):
            diff = niza[i + 1] - niza[i]

            if diff > 0:  # rise
                if diff > max_rise:
                    max_rise = diff

            elif diff < 0:  # fall
                if abs(diff) > max_fall:
                    max_fall = abs(diff)

        print(f"Valutata {key} imase najgolem porast od {max_rise} i najgolem pad od {max_fall}")

#  Потребно е да се напише Python скрипта за работа со крипто-валути. Скриптата добива на влез текстуална
# датотека во која, покрај другиот текст, се содржат вредности на крипто-валути. Скриптата треба да ја измине
# датотеката и за секоја одделна крипто-вредност да отпечати кое ќе биде најголемото зголемување од претходната
# вредност а кое ќе биде најголемото намалување од предходната вредност. На крај да се отпечатат, за секоја криптовалута одделно, најголемото зголемување на валутата и помеѓу кои вредности е истото, најголемото намалување на
# валутата и помеѓу кои вредности е истото. Крипто-валути кои се земаат во предвид се: BTC, Doge, EOS, ETH и XPM. Во
# текстот најпрво се наоѓа кратенката за крипто-валутата а потоа вредноста во долари. Сите зборови се одделени помеѓу
# себе со празно место. Доколку некоја крипто-валута ја нема во датотеката или се појавува еднаш, не се печати ништо
# за таа крипто-валута.