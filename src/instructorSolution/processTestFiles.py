import subprocess
import atexit
import time
import os
import sys
import json

baseDir = os.path.dirname(__file__)

def exitHandler():
    js = json.dumps(cache)
    cacheFile = open(baseDir + "/cachedOutcomes.json", "w")
    cacheFile.write(js)
    cacheFile.close()


atexit.register(exitHandler)


testList = open(baseDir + "/toProcess.txt", "r")
testFiles = [baseDir + "/tests/" + x.strip() for x in testList]
testList.close()

cacheFile = open(baseDir + "/cachedOutcomes.json", "r")
cache = json.loads("\n".join([x for x in cacheFile]))
cacheFile.close()

instructorClobber = baseDir + "/sample_c++/code/clobber"

print("Processing files (%d):" % (len(testFiles),))
for x in testFiles:
    print(x)
print("")

def runTest(inputLine, outputLine):
    board, toPlay, timeLimit = inputLine.split()
    #expectedWinner, expectedMove = outputLine.split()[1 : 3]

    board = [x for x in board.split(".") if len(x) > 1]
    board = ".".join(board)

    outcome = None
    if board.find("BW") == -1 and board.find("WB") == -1:
        outcome = "W" if toPlay == "B" else "B"
        return "#? %s %s %s %s" % (outcome, "None", "0.000", "0")
        

    command = instructorClobber + " %s %s %s" % (board, toPlay, "100000") 
    print(command)

    start = time.clock_gettime(time.CLOCK_MONOTONIC)
    result = subprocess.run(command, capture_output = True, shell = True)
    end = time.clock_gettime(time.CLOCK_MONOTONIC)
    output = result.stdout.decode("utf-8").rstrip("\n")

    testTime = end - start

    outcome, move, reportedTime, nodes = output.split()

    return "#? %s %s %s %s" % (outcome, move, reportedTime, nodes)


for f in testFiles:
    file = open(f, "r")

    line1 = ""
    line2 = ""

    outLines = []

    print(f)
    while True:
        line = file.readline()
        if not line:
            break
        line = line.strip()

        line2 = line

        #Run test
        if line2.find("#?") == 0:
            result = runTest(line1, line2)
            outLines.append(line1)
            outLines.append(result)

        line1 = line2

    outFile = open(f + "_processed", "w")
    outFile.write("\n".join(outLines))
    outFile.close()



    file.close()


