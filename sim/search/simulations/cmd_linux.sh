#!/usr/bin/env sh
#PATH=";D:\\Utiles\\Omnet++\\omnetpp-6.0.1-win\\bin;D:\\Utiles\\Omnet++\\omnetpp-6.0.1-win\\tools\\win32.x86_64\\opt\\mingw64\\bin;D:\\Utiles\\Omnet++\\omnetpp-6.0.1-win\\tools\\win32.x86_64\\usr\\bin;D:\\Utiles\\Omnet++\\omnetpp-6.0.1-win\\tools\\win32.x86_64\\mingw64\\bin;;D:/Utiles/Omnet++/omnetpp-6.0.1-win/ide//plugins/org.eclipse.justj.openjdk.hotspot.jre.full.stripped.win32.x86_64_17.0.4.v20220805-1047/jre/bin/server;D:/Utiles/Omnet++/omnetpp-6.0.1-win/ide//plugins/org.eclipse.justj.openjdk.hotspot.jre.full.stripped.win32.x86_64_17.0.4.v20220805-1047/jre/bin;D:\\Utiles\\Omnet++\\omnetpp-6.0.1-win\\bin;D:\\Utiles\\Omnet++\\omnetpp-6.0.1-win\\tools\\win32.x86_64\\opt\\mingw64\\bin;D:\\Utiles\\Omnet++\\omnetpp-6.0.1-win\\tools\\win32.x86_64\\mingw64\\bin;D:\\Utiles\\Omnet++\\omnetpp-6.0.1-win\\tools\\win32.x86_64\\usr\\local\\bin;D:\\Utiles\\Omnet++\\omnetpp-6.0.1-win\\tools\\win32.x86_64\\usr\\bin;D:\\Utiles\\Omnet++\\omnetpp-6.0.1-win\\tools\\win32.x86_64\\usr\\bin;C:\\Windows\\System32;C:\\Windows;C:\\Windows\\System32\\Wbem;C:\\Windows\\System32\\WindowsPowerShell\\v1.0\\;D:\\Utiles\\Omnet++\\omnetpp-6.0.1-win\\tools\\win32.x86_64\\usr\\bin\\site_perl;D:\\Utiles\\Omnet++\\omnetpp-6.0.1-win\\tools\\win32.x86_64\\usr\\bin\\vendor_perl;D:\\Utiles\\Omnet++\\omnetpp-6.0.1-win\\tools\\win32.x86_64\\usr\\bin\\core_perl;D:\\Utiles\\Omnet++\\omnetpp-6.0.1-win;"
#OMNETPP_RELEASE=omnetpp-6.0.1
#OMNETPP_IMAGE_PATH=D:\\Utiles\\Omnet++\\omnetpp-6.0.1-win\\images

EXE_NAME=COoL-TEE
LOG_DIR=results/

run_batches() 
{
    EXPERIMENT_NAME=$4
    START=$1
    END=$2
    BATCH=$3
    i=$START
    while test $i -lt $END
    do
        for j in `seq 0 $((BATCH-1))`
        do
            (echo "process $i started" && ../src/$EXE_NAME -r $i -m -u Cmdenv -c $5 -n ".;../src" omnetpp_linux.ini --record-eventlog=false --scalar-recording=false --vector-recording=false > $LOG_DIR$EXPERIMENT_NAME-$i.log && echo "process $i finished") &
            i=$(($i+1))
            if test $i -ge $END
            then
                break
            fi
        done
        echo "Waiting for processes" $(($i-$BATCH)) "to" $(($i-1)) "to finish" 
        wait $(jobs -p)
        echo "Processes" $(($i-$BATCH)) "to" $(($i-1)) "finished"
    done
}
../src/$EXE_NAME -c $5 -n ".;../src" omnetpp_linux.ini -q runs > $LOG_DIR$4-runs.log
run_batches $1 $2 $3 $4 $5
read -p "Press enter to continue"

#.\cmd.sh <run-num-start:5000> <run-num-end:6000> <batch-size:12> <exp-name:wait500> <config-name:TwoSpeedService>