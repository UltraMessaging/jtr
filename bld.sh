#!/bin/sh
# bld.sh

. ./lbm.sh

gcc -g -I$LBM_PLATFORM/include -L$LBM_PLATFORM/lib -llbm -lm -lpthread -lrt -o jtr_lbm jtr.c jtr_lbm.c

gcc -g -I$LBM_PLATFORM/include -L$LBM_PLATFORM/lib -lm -lpthread -lrt -o jtr_sock jtr.c jtr_sock.c

gcc -g -I$LBM_PLATFORM/include -L$LBM_PLATFORM/lib -lm -lpthread -lrt -o jtr_null jtr.c jtr_null.c
