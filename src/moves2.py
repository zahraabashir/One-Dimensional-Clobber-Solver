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


        comment = ""

        if ";" in line:
            comment = line[line.index(";") + 1 : ]

        line = line.upper().split()

        if len(line)


        """
            #, q, pb, pw, sm, sc, c, o, ;
             q, pb, pw, sm, sc, c, o
        """

