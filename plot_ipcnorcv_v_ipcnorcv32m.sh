#!/bin/sh
# plot_ipcnorcv_v_ipcnorcv32m.sh

cat <<__EOF__ | gnuplot

reset
set term png size 1200,960
set output "plot_ipcnorcv_v_ipcnorcv32m-1.png"
set xlabel "Historgram bucket (ns)"
set ylabel "Bucket count (log)"
set yrange [0.5:2000000]
set logscale y
set mxtics 4
load "< sed -n <tst_ipcnorcv.gp '/^#/s/^# *//p'"
title_ipcnorcv = title_1
load "< sed -n <tst_ipcnorcv32m.gp '/^#/s/^# *//p'"
set xrange [0:xrange_1]
title_ipcnorcv32m = title_1
title_combined = sprintf("%s\n\n%s", title_ipcnorcv, title_ipcnorcv32m)
set title title_combined noenhanced
plot \
"tst_ipcnorcv.gp" using 1:2 index 0 with impulses linewidth 2 linecolor 1 title "tst_ipcnorcv", \
"tst_ipcnorcv32m.gp" using 1:2 index 0 with points linewidth 2 linecolor 2 title "tst_ipcnorcv32m"

reset
set term png size 1200,960
set output "plot_ipcnorcv_v_ipcnorcv32m-2.png"
set xlabel "Historgram bucket (ns)"
set ylabel "Bucket count (log)"
set yrange [0.5:2000000]
set logscale y
set mxtics 4
load "< sed -n <tst_ipcnorcv.gp '/^#/s/^# *//p'"
title_ipcnorcv = title_3
load "< sed -n <tst_ipcnorcv32m.gp '/^#/s/^# *//p'"
set xrange [0:xrange_1]
title_ipcnorcv32m = title_3
title_combined = sprintf("%s\n\n%s", title_ipcnorcv, title_ipcnorcv32m)
set title title_combined noenhanced
plot \
"tst_ipcnorcv.gp" using 1:2 index 2 with impulses linewidth 2 linecolor 1 title "tst_ipcnorcv", \
"tst_ipcnorcv32m.gp" using 1:2 index 2 with points linewidth 2 linecolor 2 title "tst_ipcnorcv32m"

reset
set term png size 1200,960
set output "plot_ipcnorcv_v_ipcnorcv32m-3.png"
set xlabel "Historgram bucket (ns)"
set ylabel "Bucket count (log)"
set yrange [0.5:2000000]
set logscale y
set mxtics 4
load "< sed -n <tst_ipcnorcv.gp '/^#/s/^# *//p'"
title_ipcnorcv = title_5
load "< sed -n <tst_ipcnorcv32m.gp '/^#/s/^# *//p'"
set xrange [0:xrange_1]
title_ipcnorcv32m = title_5
title_combined = sprintf("%s\n\n%s", title_ipcnorcv, title_ipcnorcv32m)
set title title_combined noenhanced
plot \
"tst_ipcnorcv.gp" using 1:2 index 4 with impulses linewidth 2 linecolor 1 title "tst_ipcnorcv", \
"tst_ipcnorcv32m.gp" using 1:2 index 4 with points linewidth 2 linecolor 2 title "tst_ipcnorcv32m"

__EOF__
