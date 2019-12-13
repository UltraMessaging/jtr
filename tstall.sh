#!/bin/sh
# tstall.sh

for F in tst_*.sh; do :
  ./$F $*
done
