#!/bin/sh
# tst_lbmnosock500.sh

. ./lbm.sh

cp streaming.cfg lbm.cfg

export LBTRM_SRC_LOSS_RATE=100

./jtr_lbm -d "tst_lbmnosock500.sh" -n 500 -h 200 -g tst_lbmnosock500.gp $* >tst_lbmnosock500.txt
