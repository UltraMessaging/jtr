#!/bin/sh
# tst_lbmnosockdeftwb.sh

. ./lbm.sh

# Remove transport_lbtrm_smart_src_transmission_window_buffer_count config option.
# (let it default)
sed '/transport_lbtrm_smart_src_transmission_window_buffer_count/d' <streaming.cfg >lbm.cfg

export LBTRM_SRC_LOSS_RATE=100

./jtr_lbm -d "tst_lbmnosockdeftwb.sh" -h 200 -g tst_lbmnosockdeftwb.gp $* >tst_lbmnosockdeftwb.txt
