#!/bin/sh
# tst_clock.sh

. ./lbm.sh

# -t 2 changes timebase to clock_gettime()
./jtr_null -d "tst_clock.sh" -t 2 -h 200 -g tst_clock.gp $* >tst_clock.txt
