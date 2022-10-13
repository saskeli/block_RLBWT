with open("b_test.txt") as in_file:
    c = in_file.read(1)
    current = c
    count = 0
    while (current == c):
        count += 1
        current = in_file.read(1)
    print(count)