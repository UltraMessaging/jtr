#!/bin/sh
# tst_lbmsock.sh

. ./lbm.sh

cp streaming.cfg lbm.cfg

echo "tst_lbmsock.sh $*"
./jtr_lbm $*
