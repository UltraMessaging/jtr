#!/bin/sh
# tst_srcnosock.sh

. ./lbm.sh

cp streaming.cfg lbm.cfg
echo "source transport lbtrm" >>lbm.cfg

export LBTRM_SRC_LOSS_RATE=100

./jtr_src -d "tst_srcnosock.sh" -h 200 -g tst_srcnosock.gp $* >tst_srcnosock.txt
