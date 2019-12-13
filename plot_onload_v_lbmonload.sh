#!/bin/sh
# plot_onload_v_lbmonload.sh

cat <<__EOF__ | gnuplot

reset
set term png size 1200,960
set output "plot_onload_v_lbmonload-1.png"
set xlabel "Historgram bucket (ns)"
set ylabel "Bucket count (log)"
set yrange [0.5:1000000]
set logscale y
set mxtics 4
load "< sed -n <tst_onload.gp '/^#/s/^# *//p'"
title_onload = title_1
load "< sed -n <tst_lbmonload.gp '/^#/s/^# *//p'"
set xrange [0:xrange_1]
title_lbmonload = title_1
title_combined = sprintf("%s\n\n%s", title_onload, title_lbmonload)
set title title_combined noenhanced
plot \
"tst_onload.gp" using 1:2 index 0 with points linewidth 2 linecolor 1 title "tst_onload", \
"tst_lbmonload.gp" using 1:2 index 0 with points linewidth 2 linecolor 2 title "tst_lbmonload"

reset
set term png size 1200,960
set output "plot_onload_v_lbmonload-2.png"
set xlabel "Historgram bucket (ns)"
set ylabel "Bucket count (log)"
set yrange [0.5:1000000]
set logscale y
set mxtics 4
load "< sed -n <tst_onload.gp '/^#/s/^# *//p'"
title_onload = title_3
load "< sed -n <tst_lbmonload.gp '/^#/s/^# *//p'"
set xrange [0:xrange_3]
title_lbmonload = title_3
title_combined = sprintf("%s\n\n%s", title_onload, title_lbmonload)
set title title_combined noenhanced
plot \
"tst_onload.gp" using 1:2 index 2 with points linewidth 2 linecolor 1 title "tst_onload", \
"tst_lbmonload.gp" using 1:2 index 2 with points linewidth 2 linecolor 2 title "tst_lbmonload"

reset
set term png size 1200,960
set output "plot_onload_v_lbmonload-3.png"
set xlabel "Historgram bucket (ns)"
set ylabel "Bucket count (log)"
set yrange [0.5:1000000]
set logscale y
set mxtics 4
load "< sed -n <tst_onload.gp '/^#/s/^# *//p'"
title_onload = title_5
load "< sed -n <tst_lbmonload.gp '/^#/s/^# *//p'"
set xrange [0:xrange_5]
title_lbmonload = title_5
title_combined = sprintf("%s\n\n%s", title_onload, title_lbmonload)
set title title_combined noenhanced
plot \
"tst_onload.gp" using 1:2 index 4 with points linewidth 2 linecolor 1 title "tst_onload", \
"tst_lbmonload.gp" using 1:2 index 4 with points linewidth 2 linecolor 2 title "tst_lbmonload"

__EOF__
