import subprocess
import atexit
import time
import os

def resetColor():
    print(cReset)

atexit.register(resetColor)

cRed = "\u001b[31m"
cGreen = "\u001b[32m"
cReset = "\u001b[0m"


testFile = open("1.txt", "r")
# testFile = open("worstcases.txt", "r")

tests = []
for line in testFile:
    line = line.split(" ")
    # print(line)
    params = line[0].strip()
    result = line[1].strip()
    time1 = line[2].strip("\n")
    # print(params, result, time1)

    tests.append((params, result, time1))

testFile.close()
time_out = False

for t in tests:
    command = "./clobber " + t[0] + " " + t[1] + " 1"
    start = time.clock_gettime(time.CLOCK_MONOTONIC)

    try:
        result = subprocess.run(command, capture_output = True, shell = True, timeout=1)

    except subprocess.TimeoutExpired:
        print("TIME OUT")
        time_out = True

    end = time.clock_gettime(time.CLOCK_MONOTONIC)


    # if time_out:
    #     output = "? None"
    output = result.stdout.decode("utf-8").rstrip("\n")


    # print(command + ": ", end="")
    print(output, end="")
    print(" " + str(end - start))
