#!/bin/sh
# tst_null.sh

. ./lbm.sh

./jtr_null -d "tst_null.sh" -h 200 -g tst_null.gp $* >tst_null.txt
