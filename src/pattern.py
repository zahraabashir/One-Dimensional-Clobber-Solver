import subprocess
import time

pair_range = [1, 60]

proc = subprocess.Popen("./TheSolvers --persist".split(), stdin=subprocess.PIPE,
    stderr=subprocess.PIPE, stdout=subprocess.PIPE)


def solve_board(board, player):
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

    assert proc.poll() is None

    assert line[0 : 2] in ["B ", "W "]

    print(f"{case}")
    print(f"{line}")
    print(f"{time_end - time_start} s")
    print("")


for i in range(pair_range[0], pair_range[1] + 1):
    print(f"BW^{i}")
    solve_board("BW" * i, "B")

