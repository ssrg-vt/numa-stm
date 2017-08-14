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
set xtics 32

set title '0.1% update'
set xlabel '#core'
set ylabel 'Ops / msec'
set xrange [1:*]
plot \
  'arm/rlu.dat' using 1:2 index 0 title 'ARM' with linespoints ls 1, \
  'arm/rlu-epoch.dat' using 1:2 index 0 title 'ARM-Epoch' with linespoints ls 2, \
  '120/rlu.dat' using 1:2 index 0 title 'Xeon' with linespoints ls 3, \
  '120/rlu-epoch.dat' using 1:2 index 0 title 'Xeon-Epoch' with linespoints ls 4, \
  'xeonphi/rlu.dat' using 1:2 index 0 title 'Phi' with linespoints ls 5, \
  'xeonphi/rlu-epoch.dat' using 1:2 index 0 title 'Phi-Epoch' with linespoints ls 6

set title '1% update'
plot \
  'arm/rlu.dat' using 1:2 index 1 title 'ARM' with linespoints ls 1, \
  'arm/rlu-epoch.dat' using 1:2 index 1 title 'ARM-Epoch' with linespoints ls 2, \
  '120/rlu.dat' using 1:2 index 1 title 'Xeon' with linespoints ls 3, \
  '120/rlu-epoch.dat' using 1:2 index 1 title 'Xeon-Epoch' with linespoints ls 4, \
  'xeonphi/rlu.dat' using 1:2 index 1 title 'Phi' with linespoints ls 5, \
  'xeonphi/rlu-epoch.dat' using 1:2 index 1 title 'Phi-Epoch' with linespoints ls 6

set title '10% update'
plot \
  'arm/rlu.dat' using 1:2 index 2 title 'ARM' with linespoints ls 1, \
  'arm/rlu-epoch.dat' using 1:2 index 2 title 'ARM-Epoch' with linespoints ls 2, \
  '120/rlu.dat' using 1:2 index 2 title 'Xeon' with linespoints ls 3, \
  '120/rlu-epoch.dat' using 1:2 index 2 title 'Xeon-Epoch' with linespoints ls 4, \
  'xeonphi/rlu.dat' using 1:2 index 2 title 'Phi' with linespoints ls 5, \
  'xeonphi/rlu-epoch.dat' using 1:2 index 2 title 'Phi-Epoch' with linespoints ls 6

set title '50% update'
plot \
  'arm/rlu.dat' using 1:2 index 3 title 'ARM' with linespoints ls 1, \
  'arm/rlu-epoch.dat' using 1:2 index 3 title 'ARM-Epoch' with linespoints ls 2, \
  '120/rlu.dat' using 1:2 index 3 title 'Xeon' with linespoints ls 3, \
  '120/rlu-epoch.dat' using 1:2 index 3 title 'Xeon-Epoch' with linespoints ls 4, \
  'xeonphi/rlu.dat' using 1:2 index 3 title 'Phi' with linespoints ls 5, \
  'xeonphi/rlu-epoch.dat' using 1:2 index 3 title 'Phi-Epoch' with linespoints ls 6


set title '100% update'
plot \
  'arm/rlu.dat' using 1:2 index 4 title 'ARM' with linespoints ls 1, \
  'arm/rlu-epoch.dat' using 1:2 index 4 title 'ARM-Epoch' with linespoints ls 2, \
  '120/rlu.dat' using 1:2 index 4 title 'Xeon' with linespoints ls 3, \
  '120/rlu-epoch.dat' using 1:2 index 4 title 'Xeon-Epoch' with linespoints ls 4, \
  'xeonphi/rlu.dat' using 1:2 index 4 title 'Phi' with linespoints ls 5, \
  'xeonphi/rlu-epoch.dat' using 1:2 index 4 title 'Phi-Epoch' with linespoints ls 6
