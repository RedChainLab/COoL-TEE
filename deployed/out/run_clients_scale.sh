BATCH_SIZE=$1
SCALE_PERIOD=$2
K_REQS=$3
K_ERR=$4
K_D=$5

current_batch=0
while true; do
    # run run_clients.sh for current_batch to current_batch + BATCH_SIZE
    ./run_clients.sh $current_batch $(($current_batch + $BATCH_SIZE - 1)) $K_REQS $K_ERR $K_D
    current_batch=$(($current_batch + $BATCH_SIZE))
    sleep $SCALE_PERIOD
done