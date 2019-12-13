#!/bin/sh
# plotall.sh

for F in plot_*.sh; do :
  ./$F $*
done
