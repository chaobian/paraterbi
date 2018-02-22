#! /bin/python3


# n is number of executables run/each n consecutive lines contain 1 comparative sample
n = 3

ws = open("tmp.err","r").read().split()

accs = [0.0 for i in range(n)]


for i in range(0, len(ws), n):
    for offset in range(n):
        accs[offset] += float(ws[offset])


samples = len(ws) / float(n)
[print(acc / samples) for acc in accs]
