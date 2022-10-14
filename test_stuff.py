import sys
from collections import defaultdict

def main(path):
    d = defaultdict(int)
    c = None
    n = 0
    with open(path, 'rb') as in_file:
        while True:
            c = in_file.read(1)
            if not c:
                break 
            d[c] += 1
            n += 1
            if (n % 100000 == 0): 
                print(n, "read")
    print("at", n, c)
    for sym, count in d.items():
        print(sym, count)


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Need input file")
    else:
        main(sys.argv[1])