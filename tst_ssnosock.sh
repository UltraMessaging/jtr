#!/bin/sh
# tst_ssnosock.sh

. ./lbm.sh

cp streaming.cfg lbm.cfg
echo "source transport lbtrm" >>lbm.cfg

export LBTRM_SRC_LOSS_RATE=100

./jtr_ss -d "tst_ssnosock.sh" -h 200 -g tst_ssnosock.gp $* >tst_ssnosock.txt
