#!/bin/sh
# tst_ipcnorcv.sh

. ./lbm.sh

cp streaming.cfg lbm.cfg
echo "source transport lbtipc" >>lbm.cfg

./jtr_src -d "tst_ipcnorcv.sh" -h 200 -g tst_ipcnorcv.gp $* >tst_ipcnorcv.txt
