from random import Random
import sys

def main(runs, spread):
    chars = "ACGT"
    rand = Random()
    for _ in range(runs):
        rep = rand.randint(1, spread)
        print(rep * rand.choice(chars), end="")

if __name__ == "__main__":
    runs = 2 << 32
    spread = 1000
    if len(sys.argv) > 1:
        runs = int(sys.argv[1])
    if len(sys.argv) > 2:
        spread = int(sys.argv[2])
    main(runs, spread)