#sudo modprobe msr
while true;
do
    rdm=`shuf -i 1-10000 -n 1`
    for sleep_time_prob in `echo $@ | cut -d' ' -f2-`
    do
        prob=`echo $sleep_time_prob | cut -d"-" -f 1`
        if test $rdm -le $prob
        then
            sleep_time=`echo $sleep_time_prob | cut -d"-" -f 2`
            sleep $sleep_time
            echo "sleep_time" $sleep_time
            #sudo rdmsr -p $1 0x10
        fi
    done
done