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

def clearScreen():
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

def printBitList(bits):
    for b in bits:
        if b:
            color(green)
            print(1, end="")
        else:
            color(red)
            print(0, end="")
    color(white)

def solve(board, player):
    proc.stdin.write(bytes(f"{board} {player} 60000\n", encoding="utf-8"))
    proc.stdin.flush()
    result = proc.stdout.readline().decode("utf-8").rstrip()
    return result[0] == player

clearScreen()
cleanup()
atexit.register(cleanup)
os.mkfifo("inpipe")
proc = Popen("./TheSolvers", stdin = PIPE, stdout = PIPE, stderr = PIPE)
poll = select.poll()

summary = ""
defaultPlayer = "B"
solveChildren = False
silentMoves = False

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
                    
            

        line = inpipe.readline()

        ###Parse the current line

        if len(line) == 0:
            pipeReady = False
            continue

        line = line.lstrip().rstrip()

        if "#" in line:
            line = line[0 : line.index("#")]

        if len(line) == 0:
            continue


        comment = ""

        if ";" in line:
            comment = line[line.index(";") + 1 : ].lstrip().rstrip()
            line = line[ : line.index(";")]

        line = line.upper().split()

        if "Q" in line and line.index("Q") == 0:
            exit(0)
        if "PB" in line and line.index("PB") == 0:
            defaultPlayer = "B"
            continue
        if "PW" in line and line.index("PW") == 0:
            defaultPlayer = "W"
            continue
        if "C" in line and line.index("C") == 0:
            clearScreen()
            summary = ""
            defaultPlayer = "B"
            solveChildren = False
            silentMoves = False
            continue
        if "O" in line and line.index("O") == 0:
            print(summary)
            continue
        if "SC" in line and line.index("SC") == 0:
            solveChildren = True
            continue
        if "SM" in line and line.index("SM") == 0:
            silentMoves = True
            continue


        board = line[0]
        player = defaultPlayer

        if len(line) >= 2:
            player = line[1]

        opponent = "W" if player == "B" else "B"


        if not solveChildren:
            print("=" * 60)
            if comment != "":
                print(f"{board} ({comment})")
            else:
                print(f"{board}")


            moves = getMoves(board, player)
            children = [playMove(board, m) for m in moves]
            outcomes = [solve(c, opponent) for c in children]


            for o in outcomes:
                if o:
                    color(red)
                    summary += red
                else:
                    color(green)
                    summary += green
                char = "0" if o else "1";
                summary += char
                print(char, end="")
            color(white)
            summary += white
            print("\n")

            if comment != "":
                summary += f" ({comment})"
            summary += "\n"



            for i in range(len(moves)):
                displayColor = red
                if not outcomes[i]:
                    displayColor = green
                color(displayColor)

                if not silentMoves:
                    print(moves[i])
                    color(white)
                    for j in range(len(board)):
                        if j == moves[i][0]:
                            color(lblue)
                        elif j == moves[i][1]:
                            color(pink)
                        else:
                            color(white)
                        print(board[j], end="")
                    color(white)
                    print("")
                    color(displayColor)
                print(children[i])
                color(white)
                if not silentMoves:
                    print("")
            if silentMoves:
                print("")
                
                
                

