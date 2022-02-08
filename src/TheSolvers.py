import subprocess
import atexit
import time
import os


testFile = open("tests_small.txt", "r")
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

f = open("results.log", "w")

for t in tests:
    command = "./TheSolvers " + t[0] + " " + t[1] + " " +  t[2]
    start = time.clock_gettime(time.CLOCK_MONOTONIC)
    result = subprocess.run(command, capture_output = True, shell = True, timeout=1)
    end = time.clock_gettime(time.CLOCK_MONOTONIC)
    output = result.stdout.decode("utf-8")
    f.write(output)
    print(output, end="")
f.close()
