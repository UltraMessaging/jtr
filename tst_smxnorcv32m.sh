#!/bin/sh
# tst_smxnorcv32m.sh

. ./lbm.sh

sed <streaming.cfg >lbm.cfg 's/transport_lbtsmx_transmission_window_size.*/transport_lbtsmx_transmission_window_size 33554432/'
echo "source transport lbtsmx" >>lbm.cfg

./jtr_src -d "tst_smxnorcv32m.sh" -h 200 -g tst_smxnorcv32m.gp $* >tst_smxnorcv32m.txt
