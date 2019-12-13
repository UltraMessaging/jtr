#!/bin/sh
# plot_null.sh

cat <<__EOF__ | gnuplot

reset
set term png size 1200,960
set output "plot_null-1.png"
set xlabel "Historgram bucket (ns)"
set ylabel "Bucket count (log)"
set yrange [0.5:1000000]
set logscale y
set mxtics 4
load "< sed -n <tst_null.gp '/^#/s/^# *//p'"
set xrange [0:xrange_1]
title_combined = sprintf("%s", title_1)
set title title_combined noenhanced
plot \
"tst_null.gp" using 1:2 index 0 with impulses linewidth 2 linecolor 1 title "tst_null"

reset
set term png size 1200,960
set output "plot_null-2.png"
set xlabel "Historgram bucket (ns)"
set ylabel "Bucket count (log)"
set yrange [0.5:1000000]
set logscale y
set mxtics 4
load "< sed -n <tst_null.gp '/^#/s/^# *//p'"
set xrange [0:xrange_2]
title_combined = sprintf("%s", title_2)
set title title_combined noenhanced
plot \
"tst_null.gp" using 1:2 index 1 with impulses linewidth 2 linecolor 1 title "tst_null"

reset
set term png size 1200,960
set output "plot_null-3.png"
set xlabel "Historgram bucket (ns)"
set ylabel "Bucket count (log)"
set yrange [0.5:1000000]
set logscale y
set mxtics 4
load "< sed -n <tst_null.gp '/^#/s/^# *//p'"
set xrange [0:xrange_3]
title_combined = sprintf("%s", title_3)
set title title_combined noenhanced
plot \
"tst_null.gp" using 1:2 index 2 with impulses linewidth 2 linecolor 1 title "tst_null"

__EOF__
