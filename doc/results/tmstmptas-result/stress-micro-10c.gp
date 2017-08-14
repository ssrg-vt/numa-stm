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
set output 'stress-micro-10c.pdf'

set title 'Microbench: Stress testing for 1 cachelines'
set xlabel '#cores'
set ylabel '#acquires (1K) / sec'
set yrange [0:*]
set xrange [1:120]
set ytics nomirror
plot \
       'lock.TICKET.cacheline.1' using 1:($2/1000) index 0 title 'ticket' with linespoints ls 1, \
       'lock.TTAS.cacheline.1' using 1:($2/1000) index 0 title 'ticket' with linespoints ls 2, \
       'lock.MCS.cacheline.1' using 1:($2/1000) index 0 title 'MCS' with linespoints ls 3, \
       'lock.COHORT.cacheline.1' using 1:($2/1000) index 0 title 'Cohort' with linespoints ls 5, \
       'lock.CST.cacheline.1' using 1:($2/1000) index 0 title 'CST-MCS' with lp ls 6, \
       'lock.TMSTMPTAS.cacheline.1' using 1:($2/1000) index 0 title 'TMSTMP' with lp ls 7
