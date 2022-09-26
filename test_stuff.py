import sys
from collections import defaultdict

def main(path):
    d = defaultdict(int)
    n = 1 << 32
    n += 2
    c = None
    with open(path) as in_file:
        for _ in range(n):
            c = in_file.read(1)
            d[c] += 1
    print("at", n, c)
    print(d)


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Need input file")
    else:
        main(sys.argv[1])