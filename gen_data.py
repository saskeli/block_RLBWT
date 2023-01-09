from random import Random
import sys

def main(n):
    chars = "ACGT"
    o = 0
    rand = Random()
    while (o < n):
        rep = rand.randint(1, 16)
        print(rep * rand.choice(chars), end="")
        o += rep


if __name__ == "__main__":
    n = 2 << 32
    if len(sys.argv) > 1:
        n = int(sys.argv[1])
    main(n)