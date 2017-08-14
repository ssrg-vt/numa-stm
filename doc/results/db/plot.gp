#!/usr/bin/env gnuplot

######################################################################
# http://youinfinitesnake.blogspot.com/2011/02/attractive-scientific-plots-with.html
set terminal pdfcairo size 7in,4.2in font "Gill Sans,9" linewidth 4 rounded fontscale 1.0

# Line style for axes
set style line 80 lt rgb "#808080"

# Line style for grid
set style line 81 lt 0  # dashed
set style line 81 lt rgb "#808080"  # grey

set grid back linestyle 81
set border 3 back linestyle 80 # Remove border on top and right.  These
             # borders are useless and make it harder
             # to see plotted lines near the border.
    # Also, put it in grey; no need for so much emphasis on a border.
set xtics nomirror
set ytics nomirror

#set log x
#set mxtics 10    # Makes logscale look good.

# Line styles: try to pick pleasing colors, rather
# than strictly primary colors or hard-to-see colors
# like gnuplot's default yellow.  Make the lines thick
# so they're easy to see in small plots in papers.
set style line 1 lt rgb "#A00000" lw 2 pt 1
set style line 2 lt rgb "#00A000" lw 2 pt 6
set style line 3 lt rgb "#5060D0" lw 2 pt 2
set style line 4 lt rgb "#F25900" lw 2 pt 9
######################################################################

# set terminal pdf
set output 'plot.pdf'

# set key default
# set key left
# set key spacing 1.5
# set key samplen 3 center bottom
set key out
set xtics 30

set title 'Xeon: No contention'
set xlabel '#core'
set ylabel 'M Txns/sec'
set xrange [1:*]
set y2label 'M Aborts/sec'
set y2range [0:*]
set y2tics
plot \
  '120/silo.dat' using 1:($2/1000000) index 0 title 'Silo' with linespoints ls 1, \
  '120/silo.dat' using 1:($2/1000000) index 3 title 'Silo' with linespoints ls 8, \
  '120/tictoc.dat' using 1:($2/1000000) index 0 title 'Tictoc' with linespoints ls 2, \
  '120/hekaton.dat' using 1:($2/1000000) index 0 title 'Hekaton' with linespoints ls 5, \
  '120/eptmstmp.dat' using 1:($2/1000000) index 0 title 'EPTmstmp' with linespoints ls 6, \
  '120/hekaton.dat' using 1:($2/1000000) index 3 title 'Hekaton - clock' with linespoints ls 7


set title 'Xeon: Medium contention'
set xlabel '#core'
set ylabel 'M Txns/sec'
set xrange [1:*]
plot \
  '120/silo.dat' using 1:($2/1000000) index 1 title 'Silo' with linespoints ls 1, \
  '120/silo.dat' using 1:($3/1000000) index 1 axis x1y2 title 'Silo-Abrts' with linespoints ls 3, \
  '120/silo.dat' using 1:($2/1000000) index 4 title 'Silo' with linespoints ls 9, \
  '120/silo.dat' using 1:($3/1000000) index 4 axis x1y2 title 'Silo-Abrts' with linespoints ls 10, \
  '120/tictoc.dat' using 1:($2/1000000) index 1 title 'Tictoc' with linespoints ls 2, \
  '120/tictoc.dat' using 1:($3/1000000) index 1 axis x1y2 title 'Tictoc-Abrts' with linespoints ls 4, \
  '120/hekaton.dat' using 1:($2/1000000) index 1 title 'Hekaton' with linespoints ls 5, \
  '120/hekaton.dat' using 1:($3/1000000) index 1 axis x1y2 title 'Hekaton-Abrts' with linespoints ls 6, \
  '120/eptmstmp.dat' using 1:($2/1000000) index 1 title 'EPTmstmp' with linespoints ls 7, \
  '120/eptmstmp.dat' using 1:($3/1000000) index 1 axis x1y2 title 'EPTmstmp-Abrts' with linespoints ls 8


set title 'Xeon: High contention'
set xlabel '#core'
set ylabel 'M Txns/sec'
set xrange [1:*]
set y2tics 0.5
set ytics 0.5
set yrange[0:2.5]
set y2range[0:2.5]
plot \
  '120/silo.dat' using 1:($2/1000000) index 2 title 'Silo' with linespoints ls 1, \
  '120/silo.dat' using 1:($3/1000000) index 2 axis x1y2 title 'Silo-Abrts' with linespoints ls 3, \
  '120/silo.dat' using 1:($2/1000000) index 5 title 'Silo' with linespoints ls 11, \
  '120/silo.dat' using 1:($3/1000000) index 5 axis x1y2 title 'Silo-Abrts' with linespoints ls 12, \
  '120/tictoc.dat' using 1:($2/1000000) index 2 title 'Tictoc' with linespoints ls 2, \
  '120/tictoc.dat' using 1:($3/1000000) index 2 axis x1y2 title 'Tictoc-Abrts' with linespoints ls 4, \
  '120/tictoc.dat' using 1:($2/1000000) index 3 title 'Tictoc-RR' with linespoints ls 9, \
  '120/tictoc.dat' using 1:($3/1000000) index 3 axis x1y2 title 'Tictoc-RR-Abrts' with linespoints ls 10, \
  '120/hekaton.dat' using 1:($2/1000000) index 2 title 'Hekaton' with linespoints ls 5, \
  '120/hekaton.dat' using 1:($3/1000000) index 2 axis x1y2 title 'Hekaton-Abrts' with linespoints ls 6, \
  '120/eptmstmp.dat' using 1:($2/1000000) index 2 title 'EPTmstmp' with linespoints ls 7, \
  '120/eptmstmp.dat' using 1:($3/1000000) index 2 axis x1y2 title 'EPTmstmp-Abrts' with linespoints ls 8


set title 'Xeon: No contention'
set xlabel '#core'
set ylabel 'M Txns/sec'
set xrange [1:*]
set y2label 'M Aborts/sec'
set y2range [0:*]
set yrange [0:*]
set ytics 10
set y2tics 10
set y2tics
plot \
  '120/tictoc.dat' using 1:($2/1000000) index 0 title 'Tictoc' with linespoints ls 1, \
  '120/eptmstmp.dat' using 1:($2/1000000) index 0 title 'EPTmstmp' with linespoints ls 2, \
  '120/eptmstmp.dat' using 1:($2/1000000) index 3 title 'EPTMSTMP-mcs' with linespoints ls 3


set title 'Xeon: Medium contention'
set xlabel '#core'
set ylabel 'M Txns/sec'
set xrange [1:*]
set ytics 2
set y2tics 2
plot \
  '120/tictoc.dat' using 1:($2/1000000) index 1 title 'Tictoc' with linespoints ls 1, \
  '120/tictoc.dat' using 1:($3/1000000) index 1 axis x1y2 title 'Tictoc-Abrts' with linespoints ls 2, \
  '120/eptmstmp.dat' using 1:($2/1000000) index 1 title 'EPTmstmp' with linespoints ls 3, \
  '120/eptmstmp.dat' using 1:($3/1000000) index 1 axis x1y2 title 'EPTmstmp-Abrts' with linespoints ls 4, \
  '' using 1:($2/1000000) index 4 title 'EPTMSTMP-MCS' with linespoints ls 5, \
  '' using 1:($3/1000000) index 4 axis x1y2 title 'EPTMSTMP-MCS-Abrts' with linespoints ls 6


set title 'Xeon: High contention'
set xlabel '#core'
set ylabel 'M Txns/sec'
set xrange [1:*]
set y2tics 0.5
set ytics 0.5
set yrange[0:2.5]
set y2range[0:2.5]
plot \
  '120/tictoc.dat' using 1:($2/1000000) index 2 title 'Tictoc' with linespoints ls 1, \
  '120/tictoc.dat' using 1:($3/1000000) index 2 axis x1y2 title 'Tictoc-Abrts' with linespoints ls 2, \
  '120/eptmstmp.dat' using 1:($2/1000000) index 2 title 'EPTmstmp' with linespoints ls 3, \
  '120/eptmstmp.dat' using 1:($3/1000000) index 2 axis x1y2 title 'EPTmstmp-Abrts' with linespoints ls 4, \
  '120/eptmstmp.dat' using 1:($2/1000000) index 5 title 'EPTMSTMP-MCS' with linespoints ls 5, \
  '120/eptmstmp.dat' using 1:($3/1000000) index 5 axis x1y2 title 'EPTMSTMP-MCS-Abrts' with linespoints ls 6


set title 'ARM: No contention'
set xlabel '#core'
set ylabel 'M Txns/sec'
set xrange [1:*]
set yrange[0:*]
set ytics 5
set xtics 16
unset y2tics
unset y2label
plot \
  'arm/silo.dat' using 1:($2/1000000) index 0 title 'Silo' with linespoints ls 1, \
  'arm/tictoc.dat' using 1:($2/1000000) index 0 title 'Tictoc' with linespoints ls 2, \
  'arm/eptmstmp.dat' using 1:($2/1000000) index 0 title 'EPTmstmp' with linespoints ls 3


set title 'ARM: Medium contention'
set xlabel '#core'
set ylabel 'M Txns/sec'
set y2label 'K Aborts/sec'
set xrange [1:*]
set y2tics 1
set ytics 1
set yrange[0:3]
set y2range[0:*]
plot \
  'arm/silo.dat' using 1:($2/1000000) index 1 title 'Silo' with linespoints ls 1, \
  'arm/silo.dat' using 1:($3/1000) index 1 axis x1y2 title 'Silo-Abrts' with linespoints ls 3, \
  'arm/tictoc.dat' using 1:($2/1000000) index 1 title 'Tictoc' with linespoints ls 2, \
  'arm/tictoc.dat' using 1:($3/1000) index 1 axis x1y2 title 'Tictoc-Abrts' with linespoints ls 4, \
  'arm/eptmstmp.dat' using 1:($2/1000000) index 1 title 'EPTmstmp' with linespoints ls 7, \
  'arm/eptmstmp.dat' using 1:($3/1000) index 1 axis x1y2 title 'EPTmstmp-Abrts' with linespoints ls 8


set title 'ARM: High contention'
set xlabel '#core'
set ylabel 'M Txns/sec'
set y2label 'M Aborts/sec'
set xrange [1:*]
set yrange [0:1]
set ytics 0.2
set y2tics 0.2
plot \
  'arm/silo.dat' using 1:($2/1000000) index 2 title 'Silo' with linespoints ls 1, \
  'arm/silo.dat' using 1:($3/1000000) index 2 axis x1y2 title 'Silo-Abrts' with linespoints ls 3, \
  'arm/tictoc.dat' using 1:($2/1000000) index 2 title 'Tictoc' with linespoints ls 2, \
  'arm/tictoc.dat' using 1:($3/1000000) index 2 axis x1y2 title 'Tictoc-Abrts' with linespoints ls 4, \
  'arm/eptmstmp.dat' using 1:($2/1000000) index 2 title 'EPTmstmp' with linespoints ls 7, \
  'arm/eptmstmp.dat' using 1:($3/1000000) index 2 axis x1y2 title 'EPTmstmp-Abrts' with linespoints ls 8


set title 'Phi: No contention'
set xlabel '#core'
set ylabel 'M Txns/sec'
set xrange [1:*]
set yrange[0:*]
set ytics 5
set xtics 16
unset y2tics
unset y2label
plot \
  'phi/silo.dat' using 1:($2/1000000) index 0 title 'Silo' with linespoints ls 1, \
  'phi/silo.dat' using 1:($2/1000000) index 3 title 'Silo orig' with linespoints ls 4, \
  'phi/tictoc.dat' using 1:($2/1000000) index 0 title 'Tictoc' with linespoints ls 2, \
  'phi/eptmstmp.dat' using 1:($2/1000000) index 0 title 'EPTmstmp' with linespoints ls 3


set title 'Phi: Medium contention'
set xlabel '#core'
set ylabel 'M Txns/sec'
set y2label 'K Aborts/sec'
set xrange [1:*]
set y2tics 20
set ytics 2
set yrange[0:12]
set y2range[0:*]
plot \
  'phi/silo.dat' using 1:($2/1000000) index 1 title 'Silo' with linespoints ls 1, \
  'phi/silo.dat' using 1:($3/1000) index 1 axis x1y2 title 'Silo-Abrts' with linespoints ls 3, \
  'phi/silo.dat' using 1:($2/1000000) index 4 title 'Silo orig' with linespoints ls 9, \
  'phi/silo.dat' using 1:($3/1000) index 4 axis x1y2 title 'Silo-orig-Abrts' with linespoints ls 10, \
  'phi/tictoc.dat' using 1:($2/1000000) index 1 title 'Tictoc' with linespoints ls 2, \
  'phi/tictoc.dat' using 1:($3/1000) index 1 axis x1y2 title 'Tictoc-Abrts' with linespoints ls 4, \
  'phi/eptmstmp.dat' using 1:($2/1000000) index 1 title 'EPTmstmp' with linespoints ls 7, \
  'phi/eptmstmp.dat' using 1:($3/1000) index 1 axis x1y2 title 'EPTmstmp-Abrts' with linespoints ls 8


set title 'Phi: High contention'
set xlabel '#core'
set ylabel 'M Txns/sec'
set y2label 'M Aborts/sec'
set xrange [1:*]
set yrange [0:10]
set ytics 2
set y2tics 2
plot \
  'phi/silo.dat' using 1:($2/1000000) index 2 title 'Silo' with linespoints ls 1, \
  'phi/silo.dat' using 1:($3/1000000) index 2 axis x1y2 title 'Silo-Abrts' with linespoints ls 3, \
  'phi/silo.dat' using 1:($2/1000000) index 5 title 'Silo-orig' with linespoints ls 9, \
  'phi/silo.dat' using 1:($3/1000000) index 5 axis x1y2 title 'Silo-orig-Abrts' with linespoints ls 10, \
  'phi/tictoc.dat' using 1:($2/1000000) index 2 title 'Tictoc' with linespoints ls 2, \
  'phi/tictoc.dat' using 1:($3/1000000) index 2 axis x1y2 title 'Tictoc-Abrts' with linespoints ls 4, \
  'phi/eptmstmp.dat' using 1:($2/1000000) index 2 title 'EPTmstmp' with linespoints ls 7, \
  'phi/eptmstmp.dat' using 1:($3/1000000) index 2 axis x1y2 title 'EPTmstmp-Abrts' with linespoints ls 8
