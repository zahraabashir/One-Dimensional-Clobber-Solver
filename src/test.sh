#!/bin/bash

rm -rf experiment_data diagrams
make -j 11 && echo -ne "\e[3J" && clear

mkdir experiment_data
mkdir diagrams

DATA_DIR=experiment_data
DIAGRAM_DIR=diagrams

# CS3
python3 pattern.py rand_exp 60,66 10 0 0,1 "$DATA_DIR/0-1.csv" 60

# No ID
python3 pattern.py rand_exp 60,66 10 0 0,2 "$DATA_DIR/0-2.csv" 60

# No subgame delete
python3 pattern.py rand_exp 60,66 10 0 0,3 "$DATA_DIR/0-3.csv" 60

# No prune dominated
python3 pattern.py rand_exp 60,60 10 0 0,4 "$DATA_DIR/0-4.csv" 60

# No substitute
python3 pattern.py rand_exp 60,60 10 0 0,5 "$DATA_DIR/0-5.csv" 60

# No misc
python3 pattern.py rand_exp 60,60 10 0 0,6 "$DATA_DIR/0-6.csv" 60



for INFILE in $(ls "$DATA_DIR") ; do
    if [[ ! $(echo $INFILE | grep '.csv$') ]] ; then
        continue
    fi

    OUTFILE="${INFILE%.csv}.png"
    python3 diagram.py "$DATA_DIR/$INFILE" "$DIAGRAM_DIR/$OUTFILE"
done
