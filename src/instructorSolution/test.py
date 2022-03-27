import subprocess
import atexit
import time
import os
import sys
import json

testStats = {"totalTime": 0, "totalTests": 0, "correct": 0, "failed": 0, "wrongMove": 0}

fastMode = False
noColor = False

for f in ["-fast", "-f", "--fast", "--f"]:
    if f in sys.argv:
        fastMode = True
        break

for f in ["-nc", "--nc", "-nocolor", "--nocolor", "-noColor", "--noColor"]:
    if f in sys.argv:
        noColor = True
        break

baseDir = os.path.dirname(__file__)

cRed = "\u001b[31m"
cGreen = "\u001b[32m"
cYellow = "\u001b[93m"
cReset = "\u001b[0m"

if noColor:
    cRed = ""
    cGreen = ""
    cYellow = ""
    cReset = ""

def exitHandler():
    print(cReset,end="")
    print(testStats)

    js = json.dumps(cache)
    cacheFile = open(baseDir + "/cachedOutcomes.json", "w")
    cacheFile.write(js)
    cacheFile.close()


atexit.register(exitHandler)


testList = open(baseDir + "/tests/testlist.txt", "r")
testFiles = [baseDir + "/tests/" + x.strip() for x in testList]
testList.close()

cacheFile = open(baseDir + "/cachedOutcomes.json", "r")
cache = json.loads("\n".join([x for x in cacheFile]))
cacheFile.close()

instructorClobber = baseDir + "/sample_c++/code/clobber"

print("Test files (%d):" % (len(testFiles),))
for x in testFiles:
    print(x)

def runTest(inputLine, outputLine):
    testStats["totalTests"] += 1

    board, toPlay, timeLimit = inputLine.split()
    expectedWinner, expectedMove = outputLine.split()[1 : 3]

    command = "./TheSolvers %s %s %s" % (board, toPlay, "100") 

    start = time.clock_gettime(time.CLOCK_MONOTONIC)
    result = subprocess.run(command, capture_output = True, shell = True)
    end = time.clock_gettime(time.CLOCK_MONOTONIC)
    output = result.stdout.decode("utf-8").rstrip("\n")

    testTime = end - start
    testStats["totalTime"] += testTime

    outcome, move, reportedTime, nodes = output.split()

    #print(command)
    #print(output)

    correctness = 0
    #-1 -- fail, 0 -- bad move, 1 -- correct

    if fastMode:
        if outcome != expectedWinner:
            correctness = -1
            testStats["failed"] += 1
        else:
            correctness = 1
            testStats["correct"] += 1

    if not fastMode:
        if outcome != expectedWinner:
            correctness = -1
            testStats["failed"] += 1
        else:
            if move == expectedMove:
                correctness = 1
                testStats["correct"] += 1
            else:
                if "None" in [move, expectedMove]:
                    correctness = 0
                    testStats["wrongMove"] += 1
                else:
                    newBoard = [x for x in board]
                    a, b = [int(x) for x in move.split("-")]

                    #print(str(a) + " " + str(b))
                    #print(len(newBoard))

                    if newBoard[a] not in ["B", "W"] or newBoard[b] not in ["B", "W"] or newBoard[a] == newBoard[b]:
                        #invalid move
                        correctness = 0
                        testStats["wrongMove"] += 1
                    else:
                        newBoard[b] = newBoard[a]
                        newBoard[a] = "."
                        newBoard = "".join(newBoard)
                        newBoard = [x for x in newBoard.split(".") if len(x) > 1]
                        newBoard = ".".join(newBoard)

                        nextPlayer = "W" if toPlay == "B" else "B"

                        outcome2 = None

                        if not (newBoard.find("BW") == -1 and newBoard.find("WB") == -1):
                            key = newBoard + " " + nextPlayer
                            if key in cache.keys():
                                outcome2 = cache[key]
                            else:
                                command2 = instructorClobber + " %s %s %s" % (newBoard, nextPlayer, "1000")
                                result2 = subprocess.run(command2, capture_output = True, shell = True)
                                output2 = result2.stdout.decode("utf-8").rstrip("\n")
                                outcome2 = output2.split()[0]
                                cache[key] = outcome2
                        else:
                            outcome2 = toPlay

                        if outcome2 == expectedWinner:
                            correctness = 1
                            testStats["correct"] += 1
                        else:
                            correctness = 0
                            testStats["wrongMove"] += 1

                        


    print([cRed, cYellow, cGreen][correctness + 1], end="")
    print(command)
    print("OUT: " + output)
    print("EXP: " + " ".join(outputLine.split()[1 : ]))
    print(["[FAIL]", "[WRONG MOVE]", "[PASS]"][correctness + 1])
    print(cReset)


for f in testFiles:
    file = open(f, "r")

    line1 = ""
    line2 = ""

    while True:
        line = file.readline()
        if not line:
            break
        line = line.strip()

        line2 = line

        #Run test
        if line2.find("#?") == 0:
            runTest(line1, line2)

        line1 = line2



    file.close()


