#!/bin/sh
# tst_lbmonload.sh

. ./lbm.sh

cp streaming.cfg lbm.cfg

# Meta-config that sets lots of Onload settings to spin
#export EF_POLL_USEC=-1
export EF_SPIN_USEC=-1
export EF_UDP_SEND_SPIN=1

echo "tst_lbmonload.sh $*"
onload -v ./jtr_lbm $*
