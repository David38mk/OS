# python.md §5 — date DD-MM-YYYY valid by hand: shape, days per month, leap year
M31 = (1, 3, 5, 7, 8, 10, 12)
M30 = (4, 6, 9, 11)

def valid_date(s):
    p = s.split("-")
    if len(p) != 3 or [len(x) for x in p] != [2, 2, 4]:   # shape first: DD-MM-YYYY
        return False
    if not all(x.isdigit() for x in p):                   # before any int(), or int() crashes
        return False
    d, m, y = int(p[0]), int(p[1]), int(p[2])
    leap = y % 4 == 0 and (y % 100 != 0 or y % 400 == 0)
    if m in M31:
        top = 31
    elif m in M30:
        top = 30
    elif m == 2:
        top = 29 if leap else 28
    else:
        return False                                      # month 0 or 13+
    return 1 <= d <= top

for s in ["29-02-2024", "29-02-2023", "29-02-1900", "29-02-2000", "31-04-2024",
          "31-12-2024", "00-01-2024", "1-1-2024", "12-13-2024", "ab-cd-efgh", "2024-01-31"]:
    print(f"{s:12} {valid_date(s)}")
