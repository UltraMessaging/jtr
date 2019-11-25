#!/bin/sh
# tst_lbmnosock.sh

. ./lbm.sh

cp streaming.cfg lbm.cfg

export LBTRM_SRC_LOSS_RATE=100

echo "tst_lbmnosock.sh $*"
./jtr_lbm $*
