import sys
from random import randrange

size = int(sys.argv[1])*int(sys.argv[2])
out = ""

for p in range(size):
    red = randrange(0, 255)
    green = randrange(0, 255)
    blue = randrange(0, 255)

    num = (red << 16) + (green << 8) + blue
    out += f"{num:08}"

print(len(out))