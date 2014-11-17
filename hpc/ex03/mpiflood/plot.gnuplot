set terminal pdf
set output 'result.pdf'

set title 'MPI Bandwith'
set xlabel 'Message Size [kB]'
set ylabel 'Datarate [MB/s]'
set logscale x
set logscale y
#set xrange [1e-6:1000]
#set yrange [1e-6:1000]
set grid
set key bottom right

plot \
'./data/sync.log' using 1:3 title "sync" with linespoints, \
'./data/async.log' using 1:3 title "async" with linespoints, \
'./data/sync_local.log' using 1:3 title "sync_L" with linespoints, \
'./data/async_local.log' using 1:3 title "async_L" with linespoints

