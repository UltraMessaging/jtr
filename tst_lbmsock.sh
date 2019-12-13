#!/bin/sh
# tst_lbmsock.sh

. ./lbm.sh

cp streaming.cfg lbm.cfg

# -h 1500 expands histogram.
./jtr_lbm -d "tst_lbmsock.sh" -h 1500 -g tst_lbmsock.gp $* >tst_lbmsock.txt
