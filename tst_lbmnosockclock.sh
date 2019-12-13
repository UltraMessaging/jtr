#!/bin/sh
# tst_lbmnosockclock.sh

. ./lbm.sh

cp streaming.cfg lbm.cfg

export LBTRM_SRC_LOSS_RATE=100

# -t 2 changes timebase to clock_gettime()
./jtr_lbm -d "tst_lbmnosockclock.sh" -t 2 -h 200 -g tst_lbmnosockclock.gp $* >tst_lbmnosockclock.txt
