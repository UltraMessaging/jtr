#!/bin/sh
# tst_lbmnosock.sh

. ./lbm.sh

cp streaming.cfg lbm.cfg

export LBTRM_SRC_LOSS_RATE=100

./jtr_lbm -d "tst_lbmnosock.sh" -h 200 -g tst_lbmnosock.gp $* >tst_lbmnosock.txt
