set terminal pdfcairo
set output "EximLoad.pdf"
set title 'EximLoad'
set xlabel 'cores'
set ylabel 'messages/sec'
set yrange [0:*]
set y2label 'CPU time (microsecs/message)'
set y2tics 
set ytics nomirror
plot 'EximLoad.dat' index 0 title '' with lines linecolor 1,\
  'EximLoad.dat' index 1 axis x1y2 title ' user' with linespoints linecolor 2 pointtype 6,\
  'EximLoad.dat' index 2 axis x1y2 title ' sys' with linespoints linecolor 3 pointtype 8,\
  'EximLoad.dat' index 3 axis x1y2 title ' idle' with linespoints linecolor 4 pointtype 2
