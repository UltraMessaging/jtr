#!/bin/sh
# plot_sock_v_sssock.sh

cat <<__EOF__ | gnuplot

reset
set term png size 1200,960
set output "plot_sock_v_sssock-1.png"
set xlabel "Historgram bucket (ns)"
set ylabel "Bucket count (log)"
set yrange [0.5:2000000]
set logscale y
set mxtics 4
load "< sed -n <tst_sock.gp '/^#/s/^# *//p'"
title_sock = title_1
load "< sed -n <tst_sssock.gp '/^#/s/^# *//p'"
set xrange [0:xrange_1]
title_sssock = title_1
title_combined = sprintf("%s\n\n%s", title_sock, title_sssock)
set title title_combined noenhanced
plot \
"tst_sock.gp" using 1:2 index 0 with points linewidth 2 linecolor 1 title "tst_sock", \
"tst_sssock.gp" using 1:2 index 0 with points linewidth 2 linecolor 2 title "tst_sssock"

reset
set term png size 1200,960
set output "plot_sock_v_sssock-2.png"
set xlabel "Historgram bucket (ns)"
set ylabel "Bucket count (log)"
set yrange [0.5:2000000]
set logscale y
set mxtics 4
load "< sed -n <tst_sock.gp '/^#/s/^# *//p'"
title_sock = title_3
load "< sed -n <tst_sssock.gp '/^#/s/^# *//p'"
set xrange [0:xrange_3]
title_sssock = title_3
title_combined = sprintf("%s\n\n%s", title_sock, title_sssock)
set title title_combined noenhanced
plot \
"tst_sock.gp" using 1:2 index 2 with points linewidth 2 linecolor 1 title "tst_sock", \
"tst_sssock.gp" using 1:2 index 2 with points linewidth 2 linecolor 2 title "tst_sssock"

reset
set term png size 1200,960
set output "plot_sock_v_sssock-3.png"
set xlabel "Historgram bucket (ns)"
set ylabel "Bucket count (log)"
set yrange [0.5:2000000]
set logscale y
set mxtics 4
load "< sed -n <tst_sock.gp '/^#/s/^# *//p'"
title_sock = title_5
load "< sed -n <tst_sssock.gp '/^#/s/^# *//p'"
set xrange [0:xrange_5]
title_sssock = title_5
title_combined = sprintf("%s\n\n%s", title_sock, title_sssock)
set title title_combined noenhanced
plot \
"tst_sock.gp" using 1:2 index 4 with points linewidth 2 linecolor 1 title "tst_sock", \
"tst_sssock.gp" using 1:2 index 4 with points linewidth 2 linecolor 2 title "tst_sssock"

__EOF__
