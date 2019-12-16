#!/bin/sh
# tst_smxnorcv.sh

. ./lbm.sh

cp streaming.cfg lbm.cfg
echo "source transport lbtsmx" >>lbm.cfg

./jtr_src -d "tst_smxnorcv.sh" -h 200 -g tst_smxnorcv.gp $* >tst_smxnorcv.txt
