import subprocess
import time
import random
import select
import sys

proc = None
args = sys.argv
mcgs_version_string = "version 1.2"
reset = False
altmove = False

noid = False
altdb = False
nodelete = False
nodominance = False
uselinks = True

opt_level = None
csv_file = None
timeout_ms = 60_000

zobrist_table = []
zobrist_size = 1024
int64_max = 0xFFFFFFFFFFFFFFFF

csv_fields = [
    "board",
    "player",
    "move_count",
    "seconds",
    "opt_level",
    "diagram_id",
    "hash",
]

help_string = f"""\
    Usage: python3 {sys.argv[0]} <action>

    <action>:
        bw <minimum N> <maximum N>
        bbw <minimum N> <maximum N>
        mcgs_gen <board length> <N cases> <seed>
        random <board length> <N cases> <seed>
        rand_exp <board lengths comma separated> <N cases> <seed> <comma_separated_ablations> <out_file> <timeout>
        -h, --h, -help, --help
"""

############################################################
for i in range(zobrist_size):
    for j in range(3):
        zobrist_table.append(random.randint(0, int64_max))


assert len(zobrist_table) == 3 * zobrist_size
############################################################


def get_zobrist_char_clob(tile, idx):
    assert type(tile) is str
    assert type(idx) is int and idx >= 0
    char_idx = [".", "B", "W"].index(tile)
    return zobrist_table[idx * 3 + char_idx]


def zobrist_hash(board, player):
    val = 0
    val ^= get_zobrist_char_clob(player, 0)

    for i in range(len(board)):
        char = board[i]
        val ^= get_zobrist_char_clob(char, i + 1)

    return val


def print_help_message():
    print(help_string)
    exit(0)


def set_opt_level(level):
    assert type(level) is int
    global opt_level, altdb, uselinks, noid
    global nodelete, nodominance
    opt_level = level

    noid = False
    altdb = False
    uselinks = True
    nodelete = False
    nodominance = False

    if level == 0:
        return
    if level == 1:
        altdb = True
        return
    if level == 2:
        noid = True
        return
    if level == 3:
        nodelete = True
        return
    if level == 4:
        nodominance = True
        return
    if level == 5:
        uselinks = False
        return

    if level == 6:
        altdb = True
        noid = True
        nodelete = True
        return

    assert False

def solve_board(board, player):
    global proc, altmove, noid, altdb, uselinks

    if reset and proc is not None:
        proc.stdin.close()
        proc.wait()
        proc = None

    if proc is None:
        flags = ["--persist"]

        if altmove:
            flags.append("--altmove")

        if noid:
            flags.append("--no-id")

        if altdb:
            flags.append("--altdb")

        if not uselinks:
            flags.append("--no-links")

        if nodelete:
            flags.append("--no-delete-subgames")

        if nodominance:
            flags.append("--no-delete-dominated")

        flags = " ".join(flags)

        command = f"./TheSolvers {flags}"
        #print(command)

        proc = subprocess.Popen(command.split(),
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

    timeout = timeout_ms / 1000.0 if timeout_ms is not None else None
    ready, _, failures = select.select([proc.stdout], [], [],
                                       timeout)


    time_end = time.time_ns() / (1_000_000 * 1000)
    time_total = time_end - time_start

    assert len(failures) == 0

    if len(ready) == 0:
        proc.kill()
        proc.wait()
        return None

    assert len(ready) == 1

    stdout = ready[0]
    line = stdout.readline().decode("utf-8").strip()

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


def assert_board_format_clob(board):
    assert type(board) is str
    for tile in board:
        assert tile in ["B", "W", "."]


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
    assert proc is None

    global altmove
    altmove = True

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


def handle_random():
    global reset
    reset = True

    if len(args) != 5:
        print_help_message()

    board_len = int(args[2])
    n_cases = int(args[3])
    seed = int(args[4])

    if seed == 0:
        seed = time.time_ns()

    random.seed(seed)

    assert board_len > 0 and n_cases > 0

    total_time = 0

    for i in range(n_cases):
        board_pair = get_board_random(board_len)
        board = board_pair["clob"]

        case_b = board + " " + "B"
        case_w = board + " " + "W"

        print(f"RANDOM {i + 1} of {n_cases}")

        print(case_b)
        result_b = solve_board(board, "B")
        print_result(result_b)

        print(case_w)
        result_w = solve_board(board, "W")
        print_result(result_w)

        total_time += result_b[1]
        total_time += result_w[1]

        print("")

    print(f"Total time: {total_time}s")

def open_csv(filename):
    global csv_file
    assert csv_file is None
    assert filename is None or type(filename) is str

    if filename is None:
        filename = "random_experiment.csv"

    csv_file = open(filename, "w")

    fields = ",".join([f"\"{x}\"" for x in csv_fields])
    csv_file.write(fields + "\n")
    csv_file.flush()


def assert_csv_row_format(row):
    assert type(row) is dict
    assert set(csv_fields) == set(row.keys())

def write_csv(row):
    assert_csv_row_format(row)
    assert csv_file is not None

    fields = []
    for name in csv_fields:
        field = str(row[name])
        assert "\"" not in field
        fields.append("\"" + field + "\"")

    line = ",".join(fields)
    csv_file.write(line + "\n")


def get_opponent(player):
    assert type(player) is str and player in ["B", "W"]

    if player == "B":
        return "W"
    return "B"

def count_moves(board, player):
    assert_board_format_clob(board)
    assert type(player) is str and player in ["B", "W"]

    opp = get_opponent(player)
    count = 0

    n_tiles = len(board)
    for i in range(n_tiles):
        tile = board[i]
        if (tile != player):
            continue
        prev = board[i - 1] if i > 0 else "."
        next = board[i + 1] if i + 1 < n_tiles else "."

        count += int(prev == opp)
        count += int(next == opp)

    return count

def handle_rand_exp():
    global reset, timeout_ms
    reset = True

    if len(args) != 8:
        print_help_message()

    board_lens = [int(x) for x in args[2].split(",")]
    n_cases = int(args[3])
    seed = int(args[4])
    levels = [int(x) for x in args[5].split(",")]
    out_filename = args[6]
    timeout_ms = 1000 * int(args[7])

    assert len(board_lens) == 2 and board_lens[0] <= board_lens[1]
    board_lens = [x for x in range(board_lens[0], board_lens[1] + 1)]
    assert n_cases > 0

    if seed == 0:
        seed = time.time_ns()

    random.seed(seed)

    seen_tests = set()
    n_timeouts = 0

    open_csv(out_filename)

    i = -1
    while i + 1 < n_cases:
        i += 1
        boardlength = random.choice(board_lens)
        board = get_board_random(boardlength)["clob"]
        player = random.choice(["B", "W"])

        hash = zobrist_hash(board, player)

        if hash in seen_tests:
            i -= 1
            continue

        seen_tests.add(hash)

        move_count = count_moves(board, player)

        case = board + " " + player

        print(f"RANDOM EXPERIMENT {i + 1} of {n_cases}")
        print(case)

        results = []

        for level in levels:
            set_opt_level(level)
            result = solve_board(board, player)

            if result is None:
                print("TIMEOUT")
                n_timeouts += 1
                results.clear()
                i -= 1
                break

            duration = result[1]
            print(f"Level {level}: {duration}")

            results.append({
                "board": board,
                "player": player,
                "move_count": move_count,
                "seconds": duration,
                "opt_level": level,
                "diagram_id": 0,
                "hash": hash,
            })

        if len(results) == 0:
            continue

        for result in results:
            write_csv(result)

    log = open(out_filename + ".log", "w")
    log.write(f"Timeouts: {n_timeouts}\n")
    log.write(f"Seed: {seed}\n")
    log.close()

    print(f"Completed with {n_timeouts} timeout(s)")


def handle_mcgs_gen():
    if len(args) != 5:
        print_help_message()

    board_len = int(args[2])
    n_cases = int(args[3])
    seed = int(args[4])

    if seed == 0:
        seed = time.time_ns()

    random.seed(seed)
    assert n_cases >= 0

    with open("clob.test", "w") as outfile:
        outfile.write("{" + mcgs_version_string + "}\n")
        outfile.write("/* Auto generated from linear clobber solver */\n")
        outfile.write("[clobber_1xn]\n")

        for i in range(n_cases):
            board_pair = get_board_random(board_len)
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
    "random": handle_random,
    "rand_exp": handle_rand_exp,
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

if csv_file is not None:
    csv_file.close()
