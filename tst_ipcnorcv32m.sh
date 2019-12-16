#!/bin/sh
# tst_ipcnorcv32m.sh

. ./lbm.sh

sed <streaming.cfg >lbm.cfg 's/transport_lbtipc_transmission_window_size.*/transport_lbtipc_transmission_window_size 33554432/'
echo "source transport lbtipc" >>lbm.cfg

./jtr_src -d "tst_ipcnorcv32m.sh" -h 200 -g tst_ipcnorcv32m.gp $* >tst_ipcnorcv32m.txt
