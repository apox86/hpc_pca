set terminal pdf
set output 'result.pdf'

file1 = './data/result10.log'
file2 = './data/result50.log'
file3 = './data/result100.log'
file4 = './data/result250.log'
file5 = './data/result500.log'

set title 'MPI-Ring: Round-Trip-Time'
set xlabel 'Nodes'
set ylabel 'Total Runtime [s]'
#set logscale x
set logscale y
#set xrange [1e-6:1000]
#set yrange [1e-6:1000]
set grid
set key bottom right

plot \
file1 using 1:3 title "10 Messages" with linespoints, \
file2 using 1:3 title "50 Messages" with linespoints, \
file3 using 1:3 title "100 Messages" with linespoints, \
file4 using 1:3 title "250 Messages" with linespoints, \
file5 using 1:3 title "500 Messages" with linespoints


set title 'MPI-Ring: Round-Trip-Time'
set xlabel 'Nodes'
set ylabel 'Avg. Runtime per Msg [s]'
#set logscale x
set logscale y
#set xrange [1e-6:1000]
#set yrange [1e-6:1000]
set grid
set key bottom right

plot \
file1 using 1:4 title "10 Messages" with linespoints, \
file2 using 1:4 title "50 Messages" with linespoints, \
file3 using 1:4 title "100 Messages" with linespoints, \
file4 using 1:4 title "250 Messages" with linespoints, \
file5 using 1:4 title "500 Messages" with linespoints

