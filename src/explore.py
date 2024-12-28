from subprocess import Popen, PIPE
import os
import atexit
import select
import sys

printOnlyWinning = False

helpMessage = """\
Usage: python3 explore.py [flags]

Run the program, then write lines to named pipe \"inpipe\" to interact with this program.

Flags:
    --only-wins: Omit losing moves from output

    --no-color: Don't use color escape codes

Example lines with explanations:
    ; This is a comment
        Lines starting with ';' are comments, and are ignored

    BWBWBW.BW B
        Find moves for board "BWBWBW.BW" for player B

    clear
        Clear the screen (this should delete previous output from your terminal so that you can't see it when scrolling up)"""




green = "\033[0;32m"
red = "\033[1;31m"
white = "\033[0;0m"
lblue = "\033[1;96m"
pink = "\033[1;95m"


for arg in (sys.argv[1 : ] if len(sys.argv) > 1 else []):
    if arg == "--only-wins":
        printOnlyWinning = True
        continue

    if arg == "--no-color":
        green = ""
        red = ""
        white = ""
        lblue = ""
        pink = ""
        continue

    print(helpMessage)
    exit(-1)


def color(c):
    print(c, end="")


def cleanup():
    print(white)
    try:
        os.remove("inpipe")
    except Exception:
        pass


def getOpponent(player):
    assert player in ["B", "W"]
    if player == "B":
        return "W"
    return "B"


def isLegalBoard(board):
    for x in board:
        if x not in ['B', 'W', '.']:
            return False
    return True


def isLegalMove(board, m):
    if len(m) != 2:
        return False

    for x in m:
        if not (x >= 0 and x < len(board)):
            return False
        if board[x] not in ["B", "W"]:
            return False

    if m[0] == m[1] or board[m[0]] != getOpponent(board[m[1]]):
        return False

    return True


def clearScreen():
    #os.system("clear && printf '\e[3J'")

    # Above is invalid escape sequence according to pylsp (???)
    os.system("clear && printf '\033[3J'")


def getMoves(board, player):
    assert isLegalBoard(board)
    assert player in ["B", "W"]

    opponent = getOpponent(player)

    moves = []
    for i in range(len(board)):
        if board[i] == player:
            if i > 0 and board[i - 1] == opponent:
                moves.append([i, i - 1])
            if i + 1 < len(board) and board[i + 1] == opponent:
                moves.append([i, i + 1])

    for m in moves:
        assert isLegalMove(board, m)

    return moves


def playMove(board, m):
    assert isLegalBoard(board)
    assert isLegalMove(board, m)

    _from = m[0]
    _to = m[1]

    child = [x for x in board]
    child[_to] = child[_from]
    child[_from] = "."
    child = "".join(child)
    return child


def solve(board, player):
    assert isLegalBoard(board)
    assert player in ["B", "W"]

    # TODO assert no error, better parsing
    proc.stdin.write(f"{board} {player}\n")
    proc.stdin.flush()
    result = proc.stdout.readline().rstrip()
    exitCode = proc.poll()
    if exitCode is not None:
        print(f"Solver subprocess closed with exit code {exitCode} -- this is probably an error. Stopping...")
        exit(-1)
    return result[0] == player


clearScreen()
cleanup()
atexit.register(cleanup)
os.mkfifo("inpipe")
proc = Popen(["./TheSolvers", "--persist"], stdin = PIPE, stdout = PIPE, stderr = PIPE, text = True, bufsize = 1)
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

        line = inpipe.readline().lstrip().rstrip()

        if len(line) == 0:
            pipeReady = False
            continue

        if line.find("clear") == 0:
            clearScreen()
            continue

        if line.find(";") == 0:
            continue

        line = line.split()

        assert len(line) == 2

        board = line[0]
        player = line[1]

        assert isLegalBoard(board)
        assert player in ["B", "W"]

        opponent = getOpponent(player)

        print("=" * 20)
        print(f"{board} {player}")

        moves = getMoves(board, player)
        children = [playMove(board, m) for m in moves]
        outcomes = [solve(c, opponent) for c in children]

        assert len(outcomes) == len(moves)
        assert len(outcomes) == len(children)

        for o in outcomes:
            if o:
                color(red)
            else:
                color(green)
            char = "0" if o else "1"
            print(char, end="")
        color(white)
        print("\n")

        for i in range(len(moves)):
            move = moves[i]
            _from = move[0]
            _to = move[1]
            child = children[i]
            outcome = outcomes[i]
            moveSubtitle = "Losing move" if outcome else "Winning move"

            if outcome and printOnlyWinning:
                continue

            displayColor = red
            if not outcome:
                displayColor = green
            color(displayColor)

            print(f"{move} {moveSubtitle}")
            color(white)
            for j in range(len(board)):
                if j == _from:
                    color(lblue)
                elif j == _to:
                    color(pink)
                else:
                    color(white)
                print(board[j], end="")
            color(white)
            print("")
            color(displayColor)
            print(child)
            color(white)
            print("")
