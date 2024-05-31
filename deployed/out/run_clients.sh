K_REQS=$3
K_ERR=$4
K_D=$5
X1=0.1
WINDOW_SIZE=100
UPDATE_RATIOS_COUNT=100

for i in `seq $1 $2`; do
    (./client $i $K_REQS $K_ERR $K_D $X1 $WINDOW_SIZE $UPDATE_RATIOS_COUNT &)  1> results/latency/$i.log
done