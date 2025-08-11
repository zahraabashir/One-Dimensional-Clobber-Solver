import matplotlib.pyplot as plt
import numpy as np
import math

def get_xo_data():
    infile = open("xo-output.txt")

    n = -1
    data = []

    for line in infile:
        line = line.strip()

        caret = line.find("^")
        pre_time = line.find("B, ")
        post_time = line.find("s Move:")

        # BW^n
        if caret != -1:
            assert n == -1

            n = int(line[caret + 1 : ])
            continue

        # Result
        if pre_time != -1 or post_time != -1:
            assert pre_time != -1 and post_time != -1
            assert pre_time < post_time
            assert n != -1

            duration = float(line[pre_time + len("B, ") : post_time])
            data.append([n, duration])
            n = -1
            continue

    infile.close()
    return data


def get_xxo_data():
    infile = open("xxo-output.txt")

    n = -1
    data = []
    time_list = []

    for line in infile:
        line = line.strip()

        caret = line.find("^")

        pre_time = line.find("B, ")
        if pre_time == -1:
            pre_time = line.find("W, ")

        post_time = line.find("s Move:")

        # BBW^n
        if caret != -1:
            assert n == -1

            n = int(line[caret + 1 : ])
            continue

        # Result
        if pre_time != -1 or post_time != -1:
            assert pre_time != -1 and post_time != -1
            assert pre_time < post_time

            duration = float(line[pre_time + len("B, ") : post_time])
            time_list.append(duration)
            if (len(time_list) == 2):
                data.append([n, time_list[0], time_list[1]])
                n = -1
                time_list.clear()
            continue

    infile.close()
    return data


def make_plot(filename, data, opts):
    assert type(filename) is str and type(data) is list and type(opts) is dict

    for d in data:
        assert type(d) is list and len(d) == 2

    x_data = [d[0] for d in data]
    y_data = [d[1] for d in data]

    plt.close()
    plt.title(opts["title"])
    plt.xlabel(opts["x_name"])
    plt.ylabel(opts["y_name"])
    plt.scatter(x_data, y_data, color="blue")
    plt.savefig(filename)


def get_fit(data):
    x_data = [d[0] for d in data]
    y_data = [d[1] for d in data]
    slope, intercept = np.polyfit(x_data, y_data, 1)
    print(f"Slope: {slope}, Intercept: {intercept}")

data1 = get_xo_data()
data1 = data1[23 : 44]

for d in data1:
    d[1] = math.log(d[1], 10)

make_plot("xo_plot.png", data1, {
    "x_name": "# pairs",
    "y_name": "Log time (ms)",
    "title": "XO solve time vs pair count"
})

get_fit(data1)

data2 = get_xxo_data()
data2 = data2[7 : 24]
data2 = [[d[0], math.log(d[1] + d[2], 10)] for d in data2]

make_plot("xxo_plot.png", data2, {
    "x_name": "# groups",
    "y_name": "Log time (ms)",
    "title": "XXO solve time vs XXO group count"
})

get_fit(data2)
