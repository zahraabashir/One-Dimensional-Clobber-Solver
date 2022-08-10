from subprocess import Popen, PIPE
import os
import atexit
import select

green = "\033[0;32m"
red = "\033[1;31m" 
white = "\033[0;0m"
lblue = "\033[1;96m" 
pink = "\033[1;95m" 

def color(c):
    print(c, end="")

def cleanup():
    print(white)
    try:
        os.remove("inpipe")
    except Exception:
        pass
cleanup()

atexit.register(cleanup)

proc = Popen("./TheSolvers", stdin = PIPE, stdout = PIPE, stderr = PIPE)

os.mkfifo("inpipe")

poll = select.poll()

def clear():
    os.system("clear && printf '\e[3J'")

def getMoves(board, player):
    opponent = "B" if player == "W" else "W"

    moves = []
    for i in range(len(board)):
        if board[i] == player:
            if i > 0 and board[i - 1] == opponent:
                moves.append([i, i - 1])
            if i + 1 < len(board) and board[i + 1] == opponent:
                moves.append([i, i + 1])
    return moves

def playMove(board, m):
    child = [x for x in board]
    child[m[1]] = child[m[0]]
    child[m[0]] = "."
    child = "".join(child)
    return child

clear()

def printOutcomeVector(outcomes, deep):
    if not deep:
        for o in outcomes:
            if o:
                color(green)
                print(1, end="")
            else:
                color(red)
                print(0, end="")
    else:
        for ov in outcomes:
            printOutcomeVector(ov, False)
            print("")
    color(white)

def solve(board, player):
    proc.stdin.write(bytes(f"{board} {player} 60000\n", encoding="utf-8"))
    proc.stdin.flush()
    result = proc.stdout.readline().decode("utf-8").rstrip()
    return result[0] == player


storedOutcomeVectors = []
storedComments = []
storedPlayer = None
silentMoves = False
solveChildren = False

pipeReady = False
with open("inpipe", "r") as inpipe:
    poll.register(inpipe)
    
    while True:
        while True:
            if pipeReady:
                break

            events = poll.poll(0.1)
            for e in events:
                if e[0] == inpipe.fileno() and e[1] & select.POLLIN:
                    pipeReady = True
                    break
                    
            

        line = inpipe.readline().upper()

        if len(line) != 0 and len(line.rstrip()) == 0:
            continue

        line = line.lstrip().rstrip()

        if "#" in line and line.index("#") == 0:
            continue

        line = line.split()

        if line == []:
            pipeReady = False
            continue


        if "Q" in line and line.index("Q") == 0:
            break

        if "PB" in line and line.index("PB") == 0:
            storedPlayer = "B"
            continue

        if "PW" in line and line.index("PW") == 0:
            storedPlayer = "W"
            continue

        if "SM" in line and line.index("SM") == 0:
            silentMoves = True
            continue

        if "SC" in line and line.index("SC") == 0:
            solveChildren = True
            continue

        if "C" in line and line.index("C") == 0:
            clear()
            storedOutcomeVectors = []
            storedComments = []
            storedPlayer = None
            silentMoves = False
            solveChildren = False
            continue

        if "O" in line and line.index("O") == 0:
            print("OUTCOMES:")
            for i in range(len(storedOutcomeVectors)):
                c = storedComments[i]
                if solveChildren:
                    if c != "":
                        print(f"({c})")
                    printOutcomeVector(storedOutcomeVectors[i], True)
                
                else:
                    printOutcomeVector(storedOutcomeVectors[i], False)
                    if c != "":
                        print(f" ({c})", end="")
                    print("")
            
            continue


        comment = ""

        if ";" in line:
            comment = " ".join(line[line.index(";") + 1 : ])
            line = line[0 : line.index(";")]

        
        player = storedPlayer if not storedPlayer is None else "B"
        board = ""

        moves = []
        children = []

        if len(line) >= 2:
            player = line[1]

        opponent = "W"
        if player == "W":
            opponent = "B"

        if len(line) >= 1:
            board = line[0]


        moves = getMoves(board, player)

        for m in moves:
            child = playMove(board, m)
            children.append(child)
            

        color(white)
        print("============================================================")
        print(board, end="")
        if comment != "":
            print(f" ({comment})")
        else:
            print("")

        outcomes = []

        for c in children:
            if solveChildren:
                print(c)
                moves2 = getMoves(c, opponent)
                children2 = [playMove(c, m2) for m2 in moves2]
                oc = []
                for c2 in children2:
                    if solve(c2, player):
                        oc.append(True)
                    else:
                        oc.append(False)
                outcomes.append(oc)
                printOutcomeVector(oc, False)
                print("")
            else:
                if solve(c, opponent):
                    outcomes.append(False)
                else:
                    outcomes.append(True)
        storedOutcomeVectors.append(outcomes)
        storedComments.append(comment)

        if not solveChildren:
            printOutcomeVector(outcomes, solveChildren)
        print("")
        print("")

        for i in range(len(children)):
            if silentMoves:
                break

            displayColor = white
            if outcomes[i]:
                displayColor = green
            else:
                displayColor = red
            color(displayColor)
            print(moves[i])

            for j in range(len(board)):
                color(white)
                if j == moves[i][0]:
                    color(lblue)
                if j == moves[i][1]:
                    color(pink)
                print(board[j], end="")
            color(displayColor)
            print("")


            print(children[i])
            print("")
        color(white)




