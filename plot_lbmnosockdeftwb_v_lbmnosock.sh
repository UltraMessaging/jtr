#!/bin/sh
# plot_lbmnosockdeftwb_v_lbmnosock.sh

cat <<__EOF__ | gnuplot

reset
set term png size 1200,960
set output "plot_lbmnosockdeftwb_v_lbmnosock-1.png"
set xlabel "Historgram bucket (ns)"
set ylabel "Bucket count (log)"
set yrange [0.5:1000000]
set logscale y
set mxtics 4
load "< sed -n <tst_lbmnosockdeftwb.gp '/^#/s/^# *//p'"
title_lbmnosockdeftwb = title_1
load "< sed -n <tst_lbmnosock.gp '/^#/s/^# *//p'"
set xrange [0:xrange_1]
title_lbmnosock = title_1
title_combined = sprintf("%s\n\n%s", title_lbmnosockdeftwb, title_lbmnosock)
set title title_combined noenhanced
plot \
"tst_lbmnosockdeftwb.gp" using 1:2 index 0 with points linewidth 2 linecolor 1 title "tst_lbmnosockdeftwb", \
"tst_lbmnosock.gp" using 1:2 index 0 with impulses linewidth 2 linecolor 2 title "tst_lbmnosock"

reset
set term png size 1200,960
set output "plot_lbmnosockdeftwb_v_lbmnosock-2.png"
set xlabel "Historgram bucket (ns)"
set ylabel "Bucket count (log)"
set yrange [0.5:1000000]
set logscale y
set mxtics 4
load "< sed -n <tst_lbmnosockdeftwb.gp '/^#/s/^# *//p'"
title_lbmnosockdeftwb = title_1
load "< sed -n <tst_lbmnosock.gp '/^#/s/^# *//p'"
set xrange [0:xrange_1]
title_lbmnosock = title_1
title_combined = sprintf("%s\n\n%s", title_lbmnosockdeftwb, title_lbmnosock)
set title title_combined noenhanced
plot \
"tst_lbmnosockdeftwb.gp" using 1:2 index 2 with points linewidth 2 linecolor 1 title "tst_lbmnosockdeftwb", \
"tst_lbmnosock.gp" using 1:2 index 2 with impulses linewidth 2 linecolor 2 title "tst_lbmnosock"

reset
set term png size 1200,960
set output "plot_lbmnosockdeftwb_v_lbmnosock-3.png"
set xlabel "Historgram bucket (ns)"
set ylabel "Bucket count (log)"
set yrange [0.5:1000000]
set logscale y
set mxtics 4
load "< sed -n <tst_lbmnosockdeftwb.gp '/^#/s/^# *//p'"
title_lbmnosockdeftwb = title_1
load "< sed -n <tst_lbmnosock.gp '/^#/s/^# *//p'"
set xrange [0:xrange_1]
title_lbmnosock = title_1
title_combined = sprintf("%s\n\n%s", title_lbmnosockdeftwb, title_lbmnosock)
set title title_combined noenhanced
plot \
"tst_lbmnosockdeftwb.gp" using 1:2 index 4 with points linewidth 2 linecolor 1 title "tst_lbmnosockdeftwb", \
"tst_lbmnosock.gp" using 1:2 index 4 with impulses linewidth 2 linecolor 2 title "tst_lbmnosock"

__EOF__
