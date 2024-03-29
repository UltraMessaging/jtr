#!/bin/sh
# plot_ssnosock_v_clock.sh

cat <<__EOF__ | gnuplot

reset
set term png size 1200,960
set output "plot_ssnosock_v_clock-1.png"
set xlabel "Historgram bucket (ns)"
set ylabel "Bucket count (log)"
set yrange [0.5:2000000]
set logscale y
set mxtics 4
load "< sed -n <tst_ssnosock.gp '/^#/s/^# *//p'"
title_ssnosock = title_1
load "< sed -n <tst_clock.gp '/^#/s/^# *//p'"
set xrange [0:xrange_1]
title_clock = title_1
title_combined = sprintf("%s\n\n%s", title_ssnosock, title_clock)
set title title_combined noenhanced
plot \
"tst_ssnosock.gp" using 1:2 index 0 with points linewidth 2 linecolor 1 title "tst_ssnosock", \
"tst_clock.gp" using 1:2 index 0 with impulses linewidth 2 linecolor 2 title "tst_clock"

reset
set term png size 1200,960
set output "plot_ssnosock_v_clock-2.png"
set xlabel "Historgram bucket (ns)"
set ylabel "Bucket count (log)"
set yrange [0.5:2000000]
set logscale y
set mxtics 4
load "< sed -n <tst_ssnosock.gp '/^#/s/^# *//p'"
title_ssnosock = title_3
load "< sed -n <tst_clock.gp '/^#/s/^# *//p'"
set xrange [0:xrange_1]
title_clock = title_2
title_combined = sprintf("%s\n\n%s", title_ssnosock, title_clock)
set title title_combined noenhanced
plot \
"tst_ssnosock.gp" using 1:2 index 2 with points linewidth 2 linecolor 1 title "tst_ssnosock", \
"tst_clock.gp" using 1:2 index 1 with impulses linewidth 2 linecolor 2 title "tst_clock"

reset
set term png size 1200,960
set output "plot_ssnosock_v_clock-3.png"
set xlabel "Historgram bucket (ns)"
set ylabel "Bucket count (log)"
set yrange [0.5:2000000]
set logscale y
set mxtics 4
load "< sed -n <tst_ssnosock.gp '/^#/s/^# *//p'"
title_ssnosock = title_5
load "< sed -n <tst_clock.gp '/^#/s/^# *//p'"
set xrange [0:xrange_1]
title_clock = title_3
title_combined = sprintf("%s\n\n%s", title_ssnosock, title_clock)
set title title_combined noenhanced
plot \
"tst_ssnosock.gp" using 1:2 index 4 with points linewidth 2 linecolor 1 title "tst_ssnosock", \
"tst_clock.gp" using 1:2 index 2 with impulses linewidth 2 linecolor 2 title "tst_clock"

__EOF__
