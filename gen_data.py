from random import Random
import sys

def main(runs, spread, random_string):
    if (random_string):
        gen_random(runs, spread)
    else:
        gen_fixed(runs, spread)

def gen_fixed(runs, length):
    chars = "ACGT"
    for i in range(runs):
        print(spread * chars[i % 4], end="")

def gen_random(runs, spread):
    chars = "ACGT"
    rand = Random()
    for _ in range(runs):
        rep = rand.randint(1, spread)
        print(rep * rand.choice(chars), end="")

if __name__ == "__main__":
    runs = 2 << 32
    spread = 1000
    random_string = True
    if len(sys.argv) > 1:
        runs = int(sys.argv[1])
    if len(sys.argv) > 2:
        spread = int(sys.argv[2])
    if len(sys.argv) > 3:
        random_string = False
    
    main(runs, spread, random_string)
    