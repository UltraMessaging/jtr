#!/bin/sh
# plot_smxnorcv_v_ssnosock.sh

cat <<__EOF__ | gnuplot

reset
set term png size 1200,960
set output "plot_smxnorcv_v_ssnosock-1.png"
set xlabel "Historgram bucket (ns)"
set ylabel "Bucket count (log)"
set yrange [0.5:2000000]
set logscale y
set mxtics 4
load "< sed -n <tst_smxnorcv.gp '/^#/s/^# *//p'"
title_smxnorcv = title_1
load "< sed -n <tst_ssnosock.gp '/^#/s/^# *//p'"
set xrange [0:xrange_1]
title_ssnosock = title_1
title_combined = sprintf("%s\n\n%s", title_smxnorcv, title_ssnosock)
set title title_combined noenhanced
plot \
"tst_smxnorcv.gp" using 1:2 index 0 with impulses linewidth 2 linecolor 1 title "tst_smxnorcv", \
"tst_ssnosock.gp" using 1:2 index 0 with points linewidth 2 linecolor 2 title "tst_ssnosock"

reset
set term png size 1200,960
set output "plot_smxnorcv_v_ssnosock-2.png"
set xlabel "Historgram bucket (ns)"
set ylabel "Bucket count (log)"
set yrange [0.5:2000000]
set logscale y
set mxtics 4
load "< sed -n <tst_smxnorcv.gp '/^#/s/^# *//p'"
title_smxnorcv = title_3
load "< sed -n <tst_ssnosock.gp '/^#/s/^# *//p'"
set xrange [0:xrange_1]
title_ssnosock = title_3
title_combined = sprintf("%s\n\n%s", title_smxnorcv, title_ssnosock)
set title title_combined noenhanced
plot \
"tst_smxnorcv.gp" using 1:2 index 2 with impulses linewidth 2 linecolor 1 title "tst_smxnorcv", \
"tst_ssnosock.gp" using 1:2 index 2 with points linewidth 2 linecolor 2 title "tst_ssnosock"

reset
set term png size 1200,960
set output "plot_smxnorcv_v_ssnosock-3.png"
set xlabel "Historgram bucket (ns)"
set ylabel "Bucket count (log)"
set yrange [0.5:2000000]
set logscale y
set mxtics 4
load "< sed -n <tst_smxnorcv.gp '/^#/s/^# *//p'"
title_smxnorcv = title_5
load "< sed -n <tst_ssnosock.gp '/^#/s/^# *//p'"
set xrange [0:xrange_1]
title_ssnosock = title_5
title_combined = sprintf("%s\n\n%s", title_smxnorcv, title_ssnosock)
set title title_combined noenhanced
plot \
"tst_smxnorcv.gp" using 1:2 index 4 with impulses linewidth 2 linecolor 1 title "tst_smxnorcv", \
"tst_ssnosock.gp" using 1:2 index 4 with points linewidth 2 linecolor 2 title "tst_ssnosock"

__EOF__
