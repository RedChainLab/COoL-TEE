for i in `seq $1 $2`; do
    ./server $i &
done