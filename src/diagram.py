import csv
import sys
import math
import os
import pathlib
import shutil
import matplotlib.pyplot as plt
import numpy as np

################################################## Global variables
assert len(sys.argv) == 3
args = sys.argv[1 : ]

in_filename = args[0]
out_filename = args[1]

################################################## Helper functions


def assert_pathlike(p):
    assert type(p) is str or type(p) is pathlib.PosixPath


def file_exists(p):
    assert_pathlike(p)
    p = pathlib.Path(p)
    return p.exists()


def delete_file(p):
    assert_pathlike(p)
    p = pathlib.Path(p).absolute()

    if file_exists(p):
        shutil.rmtree(p)


def create_dir(p):
    assert_pathlike(p)
    p = pathlib.Path(p).absolute()

    assert not file_exists(p)
    os.mkdir(p)
    assert file_exists(p)


def assert_opt_labels_format(opt_labels):
    assert type(opt_labels) is list
    n_fields = 4
    skip = [0, 3]

    for ol in opt_labels:
        assert type(ol) is list and len(ol) == n_fields
        assert type(ol[0]) is int

        for i in range(len(ol)):
            if i in skip:
                continue
            assert type(ol[i]) is str



def assert_diagram_info_format(info):
    assert type(info) is dict

    exp_fields = [
        "diagram_id",
        "title",
        "file_name",
        "x_label",
        "y_label",
        "x_field",
        "y_field",
        "opt_labels",
    ]

    assert set(exp_fields) == set(info.keys())

    for key in exp_fields:
        val = info[key]

        if (key == "diagram_id"):
            assert type(val) is int
        elif key == "opt_labels":
            assert_opt_labels_format(val)
        else:
            assert type(val) is str

class WrappedReader:
    def __init__(self, file_name):
        assert file_exists(file_name)

        self._file = open(file_name, "r")
        self._reader = csv.DictReader(self._file)

    def is_closed(self):
        return self._reader is None or self._file is None

    def reader(self):
        assert not self.is_closed()
        return self._reader

    def close(self):
        assert not self.is_closed()
        self._reader = None

        self._file.close()
        self._file = None


def make_diagram_abs_delta(info):
    assert_diagram_info_format(info)
    plt.close()

    # Info fields
    opt_labels = info["opt_labels"]
    opt_ids = [ol[0] for ol in opt_labels]
    diagram_id = info["diagram_id"]
    x_label = info["x_label"]
    y_label = info["y_label"]
    x_field = info["x_field"]
    y_field = info["y_field"]
    title = info["title"]
    file_name = info["file_name"]


    # Read data from file
    wrapped = WrappedReader(in_filename)
    reader = wrapped.reader()

    data = {}

    for line in reader:
        if int(line["diagram_id"]) != diagram_id:
            continue

        opt_level = int(line["opt_level"])
        if opt_level not in opt_ids:
            continue

        x_val = int(line["hash"])
        y_val = float(line[y_field])

        group = data.get(x_val)
        if group is None:
            group = {}
            data[x_val] = group
        group[opt_level] = y_val

    wrapped.close()

    # Sort data
    data_list = [data[hash] for hash in data]
    for group in data_list:
        vals = [group[opt_id] for opt_id in group]
        low = min(vals)
        high = max(vals)
        #diff = abs(low - high)
        diff = high
        group["diff"] = diff

    data_list.sort(key=lambda x: x["diff"])

    x_data = [i for i in range(len(data_list))]
    for ol in opt_labels:
        ol_id, ol_name, ol_color, ol_marker = ol
        y_data = [group.get(ol_id) for group in data_list]
        y_data = np.array([x for x in y_data if x is not None])
        if len(y_data) == 0:
            continue

        plt.scatter(x_data, y_data, label=ol_name, color=ol_color,
                    marker=ol_marker)

    plt.legend()
    plt.title(title)
    plt.xlabel(x_label)
    plt.ylabel(y_label)
    plt.savefig(out_filename)
    #plt.show()

def make_diagram(info):
    assert_diagram_info_format(info)
    plt.close()

    # Info fields
    opt_labels = info["opt_labels"]
    diagram_id = info["diagram_id"]
    x_label = info["x_label"]
    y_label = info["y_label"]
    x_field = info["x_field"]
    y_field = info["y_field"]
    title = info["title"]
    file_name = info["file_name"]


    # Read data from file
    wrapped = WrappedReader(in_file_name)
    reader = wrapped.reader()

    data_pairs = {opt_id: [[], []] for opt_id in [ol[0] for ol in opt_labels]}

    for line in reader:
        if int(line["diagram_id"]) != diagram_id:
            continue

        x_val = int(line[x_field])
        y_val = float(line[y_field])
        opt_level = int(line["opt_level"])

        x_data, y_data = data_pairs[opt_level]
        x_data.append(x_val)
        y_data.append(y_val)

    wrapped.close()

    # Make plots
    for ol in opt_labels:
        ol_id, ol_name, ol_color, ol_marker = ol
        x_data, y_data = data_pairs[ol_id]

        plt.scatter(x_data, y_data, label=ol_name, color=ol_color,
                    marker=ol_marker)

    plt.legend()
    plt.title(title)
    plt.xlabel(x_label)
    plt.ylabel(y_label)
    plt.savefig(out_filename)
    #plt.show()




################################################## Main script
# Initialize file system stuff
assert file_exists(in_filename)

# Read data
diagram_info_list = [
    {
        "diagram_id": 0,

        "title": "Time vs Test Number",
        "file_name": "diagram2.png",

        "x_label": "Test Number (sorted by increasing max time)",
        "y_label": "Time (seconds)",

        "x_field": "move_count",
        "y_field": "seconds",

        "opt_labels": [

            [0, "Default", "green", "1"],

            [1, "Complexity Score 3", "blue", "2"],
            [2, "No ID", "blue", "2"],
            [3, "No Subgame Delete", "blue", "2"],
            [4, "No Prune Dominated", "blue", "2"],
            [5, "No Substitute", "blue", "2"],
            [6, "No Misc", "blue", "2"],
        ],
    },
]


#make_diagram(diagram_info_list[0])
make_diagram_abs_delta(diagram_info_list[0])

#for info in diagram_info_list:
#    make_diagram_abs_delta(info)
