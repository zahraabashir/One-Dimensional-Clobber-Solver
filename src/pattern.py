import subprocess
import time
import random
import sys

proc = None
args = sys.argv
mcgs_version_string = "version 1.2"

help_string = f"""\
    Usage: python3 {sys.argv[0]} <action>

    <action>:
        bw <minimum N> <maximum N>
        bbw <minimum N> <maximum N>
        mcgs_gen <N cases>
        -h, --h, -help, --help
"""


def print_help_message():
    print(help_string)
    exit(0)


def solve_board(board, player):
    global proc
    if proc is None:
        proc = subprocess.Popen("./TheSolvers --persist".split(),
                                stdin=subprocess.PIPE,
                                stderr=subprocess.PIPE, stdout=subprocess.PIPE)

    assert type(board) is str
    assert type(player) is str

    assert len(board) > 0
    assert len(player) == 1

    for c in board:
        assert c in ["B", "W", "."]
    assert player in ["B", "W"]

    case = f"{board} {player}"

    assert proc.poll() is None

    time_start = time.time_ns() / (1_000_000 * 1000)

    proc.stdin.write(bytes(case + "\n", "utf-8"))
    proc.stdin.flush()

    line = proc.stdout.readline().decode("utf-8").strip()

    time_end = time.time_ns() / (1_000_000 * 1000)
    time_total = time_end - time_start

    assert proc.poll() is None

    assert line[0 : 2] in ["B ", "W "]
    move = line.split()[1]
    return [line[0], time_total, move]

def print_result(result, expected=None):
    assert type(result) is list and len(result) == 3
    assert type(result[0]) is str and type(result[1]) is float
    assert type(result[2]) is str

    print(f"{result[0]}, {result[1]}s Move: {result[2]}")
    if expected is not None and result[0] != expected:
        print("UNEXPECTED RESULT")

def assert_board_format(board):
    assert type(board) is list
    for tile in board:
        assert type(tile) is int
        assert 0 <= tile and tile <= 2


def assert_int_interval(low, high):
    assert type(low) is int and low > 0
    assert type(high) is int and high > 0 and (low <= high)


def board_to_mcgs(board):
    assert_board_format(board)
    return "".join([['.', 'X', 'O'][tile] for tile in board])


def board_to_clob(board):
    assert_board_format(board)
    return "".join([['.', 'B', 'W'][tile] for tile in board])


def board_to_pair(board):
    assert_board_format(board)

    return {
        "mcgs": board_to_mcgs(board),
        "clob": board_to_clob(board),
    }


def get_board_bw(n):
    assert type(n) is int
    assert n > 0

    board = [1, 2] * n
    return board_to_pair(board)


def get_board_bbw(n):
    assert type(n) is int
    assert n > 0

    board_main = [1, 1, 2] * n
    board_extra = [2, 2, 1, 0] * ((n + 1) // 2)
    board_extra.pop()
    board = board_main + [0] + board_extra

    return board_to_pair(board)


def get_board_random(n):
    assert type(n) is int and n >= 0
    board = [random.randint(1, 2) for i in range(n)]
    return board_to_pair(board)


def get_win_string(winning_player, to_play):
    assert type(winning_player) is str and type(to_play) is str
    assert winning_player in ["B", "W"]
    assert to_play in ["B", "W"]

    return "win" if (winning_player == to_play) else "loss"


def handle_bw():
    if len(args) != 4:
        print_help_message()

    min_n = int(args[2])
    max_n = int(args[3])
    assert_int_interval(min_n, max_n)

    for i in range(min_n, max_n + 1):
        board_pair = get_board_bw(i)
        board = board_pair["clob"]
        case = board + " " + "B"

        print(f"BW^{i}")

        print(case)
        result = solve_board(board, "B" if i != 3 else "W")
        print_result(result, "B")

        print("")


def handle_bbw():
    if len(args) != 4:
        print_help_message()

    min_n = int(args[2])
    max_n = int(args[3])
    assert_int_interval(min_n, max_n)

    for i in range(min_n, max_n + 1):
        board_pair = get_board_bbw(i)
        board = board_pair["clob"]

        case_b = board + " " + "B"
        case_w = board + " " + "W"

        print(f"BBW^{i}")

        print(case_b)
        result_b = solve_board(board, "B")
        print_result(result_b, "W")

        print(case_w)
        result_w = solve_board(board, "W")
        print_result(result_w, "B")

        print("")


def handle_mcgs_gen():
    if len(args) != 3:
        print_help_message()

    n_cases = int(args[2])
    assert n_cases >= 0

    with open("clob.test", "w") as outfile:
        outfile.write("{" + mcgs_version_string + "}\n")
        outfile.write("/* Auto generated from linear clobber solver */\n")
        outfile.write("[clobber_1xn]\n")

        for i in range(n_cases):
            board_pair = get_board_random(33)
            board_clob = board_pair["clob"]
            board_mcgs = board_pair["mcgs"]

            result_b = solve_board(board_clob, "B")
            result_w = solve_board(board_clob, "W")

            win_b = get_win_string(result_b[0], "B")
            win_w = get_win_string(result_w[0], "W")

            mcgs_out = board_mcgs + " {"
            mcgs_out += f"B {win_b}, W {win_w}"
            mcgs_out += "}\n"
            outfile.write(mcgs_out)


actions = {
    "bw": handle_bw,
    "bbw": handle_bbw,
    "mcgs_gen": handle_mcgs_gen,
}

def list_has_help_flag(l):
    assert type(l) is list
    for x in l:
        if x in ["-h", "-help", "--h", "--help"]:
            return True
    return False


if len(args) < 2 or list_has_help_flag(args):
    print_help_message()

action = args[1]

if action not in actions:
    print_help_message()

action_fn = actions[action]
action_fn()
