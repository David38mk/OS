#!/bin/bash

# Задача 2. Користејќи ја grep наредбата, да се најдат сите .c датотеки во тековниот директориум.

# samo tekoven direktorium (bez poddirektoriumi)
ls -1 | grep -E '\.c$'

# varijanta so detali:
# ls -la | grep -E '\.c$'
