shapes = []
ids = []

def genShapes(prev, shape, remaining):
    if prev == -1:
        for i in range(2, remaining + 1):
            s = [i]
            shapes.append(s)
            genShapes(i, s, remaining - i - 1)

    if remaining <= 0:
        return


    for i in range(2, min(remaining + 1, prev + 1)):
        s = shape + [i]
        shapes.append(s)
        genShapes(i, s, remaining - i - 1)

def countShape(shape):
    totalBits = sum(shape)
    count = 0
    minGame = 0
    maxGame = 2**totalBits - 1
    for game in range(minGame, maxGame + 1):
        count += 1
    return count

def shapeID(shape):
    val = 0
    base = 1
    for i in range(len(shape)):
        val += shape[i] * base
        base *= 16
    return val

def genShapeIDs():
    for s in shapes:
        ids.append(shapeID(s))

#shapes = [[i] for i in range(2, 17)]
genShapes(-1, [], 16)
genShapeIDs()

total = 0
for i in range(len(shapes)):
    s = shapes[i]
    num = ids[i]
    c = countShape(s)
    print(f"{s} ({c}) #{num}")
    total += c


print(total)

print(len(shapes))

#866924 (new format)
#131068 (old format)
