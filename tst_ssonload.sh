#!/bin/sh
# tst_ssonload.sh

. ./lbm.sh

cp streaming.cfg lbm.cfg
echo "source transport lbtrm" >>lbm.cfg

# Meta-config that sets lots of Onload settings to spin
#export EF_POLL_USEC=-1
export EF_SPIN_USEC=-1
export EF_UDP_SEND_SPIN=1

# -h 1000 expands histogram.
onload ./jtr_ss -d "tst_ssonload.sh" -h 1000 -g tst_ssonload.gp $* >tst_ssonload.txt
