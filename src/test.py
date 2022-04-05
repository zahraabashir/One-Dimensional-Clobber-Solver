import subprocess
import atexit
import time

def resetColor():
    print(cReset)

atexit.register(resetColor)

cRed = "\u001b[31m"
cGreen = "\u001b[32m"
cReset = "\u001b[0m"


# testFile = open("tests.txt", "r") 
testFile = open("tests_hard.txt", "r")
# testFile = open("tests_small.txt", "r")

tests = []
for line in testFile:
    line = line.split("-")

    params = line[0].strip()
    result = line[1].strip().strip("\n") == "True"

    toPlay = params.split()[1].strip().strip("\n")
    if result:
        result = toPlay
    else:
        result = "W" if toPlay == "B" else "B"
    tests.append((params, result))

testFile.close()


for t in tests:
    command = "./TheSolvers " + t[0] + " 10000000000"
    start = time.clock_gettime(time.CLOCK_MONOTONIC)
    result = subprocess.run(command, capture_output = True, shell = True)
    end = time.clock_gettime(time.CLOCK_MONOTONIC)

    output = result.stdout.decode("utf-8").rstrip("\n")
    # print(output)
    if output[0] == t[1]:
        print(cGreen, end="")
    else:
        print(cRed, end="")

    # print(command + ": ", end="")
    print(output, end="")
    print(" " + str(end - start))

    print(cReset,end="")
