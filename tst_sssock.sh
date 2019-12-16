#!/bin/sh
# tst_sssock.sh

. ./lbm.sh

cp streaming.cfg lbm.cfg
echo "source transport lbtrm" >>lbm.cfg

# -h 1500 expands histogram.
./jtr_ss -d "tst_sssock.sh" -h 1500 -g tst_sssock.gp $* >tst_sssock.txt
