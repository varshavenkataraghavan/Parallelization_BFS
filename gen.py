import random

n = 10
density = 0.15

with open("input.txt", "w") as f:
    f.write(f"{n}\n0\n")
    for i in range(n):
        row = [0]*n
        if i > 0:
            row[i - 1] = 1
        if i < n - 1:
            row[i + 1] = 1
        for j in range(i + 2, n):
            if random.random() < density:
                row[j] = 1
        for j in range(i):
            if row[j] == 1:
                row[i] = 1
        f.write(" ".join(map(str, row)) + "\n")
