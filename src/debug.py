from subprocess import Popen, PIPE

inFile = open("out.txt", "r")
cases = [x.rstrip() + "\n" for x in inFile if x.find("EQ:") != -1]
inFile.close()

cases = [x.replace("EQ: ", "") for x in cases]

for i in range(len(cases)):
    board = cases[i]
    print(f"{i}/{len(cases)}")

    chunks = [x for x in board.rstrip().split(".") if (x.find("WB") != -1 or x.find("BW") != -1)]
    if len(chunks) > 0:
        game = ".".join(chunks)

        proc1 = Popen(["./instructorSolution/sample_c++/code/clobber", game, "B", "500"] , stdin = PIPE, stdout = PIPE, stderr = PIPE)

        out1 = proc1.stdout.readline().decode("utf-8")


        proc2 = Popen(["./instructorSolution/sample_c++/code/clobber", game, "W", "500"] , stdin = PIPE, stdout = PIPE, stderr = PIPE)
        
        out2 = proc2.stdout.readline().decode("utf-8")

        if out1[0] != "W" or out2[0] != "B":
            print(game)
            print(out1)
            print(out2)
            print()


