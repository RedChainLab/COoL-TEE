IP=$1
BEGIN=$2
END=$3
REPETITIONS=$4
BATCH=10
for i in $(seq $BEGIN $REPETITIONS $(($END-1))); do
    for j in $(seq 0 $BATCH $(($REPETITIONS-1))); do
        for k in $(seq 0 $(($BATCH-1))); do
            run=$(($i+$j+$k))
            if [ $run -ge $END ]; then
                break
            fi
            echo "Fetching run data $run repetition $(($j+$k))"
            scp $IP:~/cooltee/sim/search/simulations/results/all/all-$run-$(($j+$k))*.csv results/all/ &
        done
        wait $(jobs -p)
    done
done