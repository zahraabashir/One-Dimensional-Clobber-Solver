import random
import subprocess
import atexit
import time

number_list = ['W', 'B', '.', 'W', 'B']
player = ['W', 'B']
# random item from list
num = 2
number_of_tests = 301
random.seed(10)

f = open("new_test.txt", "w")


for i in range(number_of_tests):
    s = ""
    for j in range(num):
        s += random.choice(number_list)
    p = random.choice(player)
    s += " "
    s += p
    if (i %5==0):
        num+=1
    # print(s)
    f.write(s+"\n")
f.close()


testFile = open("new_test.txt", "r")
tests = []
for line in testFile:

    # params = line[0].strip()
    tests.append(str(line.strip("\n")))
    print(line.strip("\n"))

all_s = time.clock_gettime(time.CLOCK_MONOTONIC)

outputs = []
new = open("final_test.txt", "w")
index = 0
for t in tests:
    command = "./TheSolvers " + str(t) + " 200"
    # start = time.clock_gettime(time.CLOCK_MONOTONIC)
    result = subprocess.run(command, capture_output = True, shell = True)
    # end = time.clock_gettime(time.CLOCK_MONOTONIC)

    output = result.stdout.decode("utf-8").rstrip("\n")
    # print(output[0])
    outputs.append(output[0])
    if output[0] == t[-1]:
        new.write(t+ " - True \n")
    else:
        new.write(t+ " - False \n")
    print(index)
    index +=1

new.close()

all_e = time.clock_gettime(time.CLOCK_MONOTONIC)

print("TOTAL TIME WITHOUT DATABASE:\t")
print(all_e-all_s)
print("\n")
