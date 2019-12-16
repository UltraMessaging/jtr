#!/bin/sh
# tst_ssnosockclock.sh

. ./lbm.sh

cp streaming.cfg lbm.cfg
echo "source transport lbtrm" >>lbm.cfg

export LBTRM_SRC_LOSS_RATE=100

# -t 2 changes timebase to clock_gettime()
./jtr_ss -d "tst_ssnosockclock.sh" -t 2 -h 200 -g tst_ssnosockclock.gp $* >tst_ssnosockclock.txt
