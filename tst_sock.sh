#!/bin/sh
# tst_sock.sh

. ./lbm.sh

# Use the LBM config file to specify the multicast socket parameters.
D=`sed -n <streaming.cfg '/transport_lbtrm_destination_port/s/^.*_port //p'`
G=`sed -n <streaming.cfg '/transport_lbtrm_multicast_address_low/s/^.*_low //p'`
I=`sed -n <streaming.cfg '/resolver_multicast_interface/s/^.*_interface //p'`
T=15

# If the lbm config file specifies the multicast interface as a "network"
# (e.g. 10.29.4.0/24), find a matching interface.
NETWORK=`echo $I | sed '/\.0\/[0-9]/s/\.0\/[0-9]*/./'`
if [ -n "$NETWORK" ]; then :
  I=`ifconfig | sed -n "/$NETWORK/s/ *inet \([0-9.]*\) .*/\1/p"`
fi

echo "tst_sock.sh -D $D -G $G -I $I -T $T $*"
./jtr_sock -D $D -G $G -I $I -T $T $*
