#!/bin/sh
# tst_null.sh

. ./lbm.sh

cp streaming.cfg lbm.cfg

echo "tst_null.sh $*"
./jtr_null $*
