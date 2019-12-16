#!/bin/sh
# tst_ssnosock32m.sh

. ./lbm.sh

# Remove transport_lbtrm_smart_src_transmission_window_buffer_count config option.
# (let it default; 1K msgs produces 32MB buffer)
sed '/transport_lbtrm_smart_src_transmission_window_buffer_count/d' <streaming.cfg >lbm.cfg
echo "source transport lbtrm" >>lbm.cfg

export LBTRM_SRC_LOSS_RATE=100

./jtr_ss -d "tst_ssnosock32m.sh" -h 200 -g tst_ssnosock32m.gp $* >tst_ssnosock32m.txt
