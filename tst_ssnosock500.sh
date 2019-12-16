#!/bin/sh
# tst_ssnosock500.sh

. ./lbm.sh

cp streaming.cfg lbm.cfg
echo "source transport lbtrm" >>lbm.cfg

export LBTRM_SRC_LOSS_RATE=100

./jtr_ss -d "tst_ssnosock500.sh" -n 500 -h 200 -g tst_ssnosock500.gp $* >tst_ssnosock500.txt
