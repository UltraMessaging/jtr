#!/bin/sh
# plot_onload_v_null.sh

cat <<__EOF__ | gnuplot

reset
set term png size 1200,960
set output "plot_onload_v_null-1.png"
set xlabel "Historgram bucket (ns)"
set ylabel "Bucket count (log)"
set yrange [0.5:2000000]
set logscale y
set mxtics 4
load "< sed -n <tst_onload.gp '/^#/s/^# *//p'"
set xrange [0:xrange_1]
title_combined = sprintf("%s\n\n%s", title_1, title_2)
set title title_combined noenhanced
plot \
"tst_onload.gp" using 1:2 index 0 with points linewidth 2 linecolor 1 title "tst_onload", \
"tst_onload.gp" using 1:2 index 1 with impulses linewidth 2 linecolor 2 title "tst_onload (null loop)"

reset
set term png size 1200,960
set output "plot_onload_v_null-2.png"
set xlabel "Historgram bucket (ns)"
set ylabel "Bucket count (log)"
set yrange [0.5:2000000]
set logscale y
set mxtics 4
load "< sed -n <tst_onload.gp '/^#/s/^# *//p'"
set xrange [0:xrange_3]
title_combined = sprintf("%s\n\n%s", title_3, title_4)
set title title_combined noenhanced
plot \
"tst_onload.gp" using 1:2 index 2 with points linewidth 2 linecolor 1 title "tst_onload", \
"tst_onload.gp" using 1:2 index 3 with impulses linewidth 2 linecolor 2 title "tst_onload (null loop)"

reset
set term png size 1200,960
set output "plot_onload_v_null-3.png"
set xlabel "Historgram bucket (ns)"
set ylabel "Bucket count (log)"
set yrange [0.5:2000000]
set logscale y
set mxtics 4
load "< sed -n <tst_onload.gp '/^#/s/^# *//p'"
set xrange [0:xrange_5]
title_combined = sprintf("%s\n\n%s", title_5, title_6)
set title title_combined noenhanced
plot \
"tst_onload.gp" using 1:2 index 4 with points linewidth 2 linecolor 1 title "tst_onload", \
"tst_onload.gp" using 1:2 index 5 with impulses linewidth 2 linecolor 2 title "tst_onload (null loop)"

__EOF__
