#!/bin/sh
# bld.sh

. ./lbm.sh

rm -f jtr_null jtr_sock jtr_ss jtr_src

gcc -g -Wall -pedantic -std=gnu99 -I$LBM_INCLUDE -L$LBM_PLATFORM/lib -lm -lpthread -lrt -o jtr_null jtr.c jtr_null.c

gcc -g -Wall -pedantic -std=gnu99 -I$LBM_INCLUDE -L$LBM_PLATFORM/lib -lm -lpthread -lrt -o jtr_sock jtr.c jtr_sock.c

gcc -g -Wall -pedantic -std=gnu99 -I$LBM_INCLUDE -L$LBM_PLATFORM/lib -llbm -lm -lpthread -lrt -o jtr_ss jtr.c jtr_ss.c

gcc -g -Wall -pedantic -std=gnu99 -I$LBM_INCLUDE -L$LBM_PLATFORM/lib -llbm -lm -lpthread -lrt -o jtr_src jtr.c jtr_src.c

gcc -g -Wall -pedantic -std=gnu99 -I$LBM_INCLUDE -L$LBM_PLATFORM/lib -llbm -lm -lpthread -lrt -o jtr_smx jtr.c jtr_smx.c
