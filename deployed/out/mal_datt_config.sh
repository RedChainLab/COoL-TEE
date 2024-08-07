#!/bin/sh
COOLTEE_HON_REQ_SUBNET="10.0.3.0/24"
ATTACK_DELAY="50ms"

RED='\033[0;31m'
YEL='\033[1;33m'
PINK='\033[1;35m'
NC='\033[0m' # No Color

for arg
do
    case $arg in
        --help|-h)
            echo "${PINK}Usage: $0 [{--help|-h} | [{-hs|--hon-subnet}=<IP_MASK>] [{-ms|--mal-subnet}=<IP_MASK>] [--delay=<DELAY>] | --rm]${NC}"
            echo "  --help, -h          Display this help message."
            echo "  {-s|--subnet}=<IP_MASK>  The mask of the honest requester's subnet to which the malicious delay attack should be applied. Default: ${COOLTEE_HON_REQ_SUBNET}"
            echo "  {-d|--delay}=<DELAY>     The delay to be applied to the packets. Default: ${ATTACK_DELAY}"
            echo "  --rm                Remove a previous malicious delay attack configuration."
            echo "  <no arguments>      Apply the malicious delay attack with the default configuration."
            exit 0
    esac
done

for arg
do
    case $arg in
        --rm)
            echo "${PINK}Disabling the malicious delay attack${NC}"
            if sudo iptables --table mangle --check OUTPUT -j MARK_PACKETS > /dev/null 2>&1
            then
                echo "${YEL}Disabling found iptables configuration.${NC}"
                sudo iptables -t mangle -F MARK_PACKETS
                sudo iptables -t mangle -D OUTPUT -j MARK_PACKETS
                sudo iptables -t mangle -X MARK_PACKETS
                echo "${YEL}Disabling found iptables configuration.${NC}"
            else
                echo "${YEL}No iptables configuration found.${NC}"
            fi

            if sudo tc qdisc show dev eth0 | grep -q "qdisc netem 10:" > /dev/null 2>&1
            then
                echo "${YEL}Disabling found tc configuration.${NC}"
                sudo tc qdisc del dev eth0 root
                echo "${YEL}Disabled found tc configuration.${NC}"
            else
                echo "${YEL}No tc configuration found.${NC}"
            fi
            exit 0
            ;;
    esac
done

for arg
do
    case $arg in
        --subnet=*|-s=*)
            COOLTEE_HON_REQ_SUBNET="${arg#*=}"
            ;;
        --delay=*)
            ATTACK_DELAY="${arg#*=}"
            ;;
    esac
done
echo "${PINK}Malicious delay attack configuration:${NC}"
echo "COOLTEE_HON_REQ_SUBNET: $COOLTEE_HON_REQ_SUBNET"
echo "ATTACK_DELAY: $ATTACK_DELAY"

echo "${PINK}Checking previous configuration.${NC}"
if sudo iptables --table mangle --check OUTPUT -j MARK_PACKETS > /dev/null 2>&1
then
    echo "${YEL}Previous iptables configuration found. Removing it.${NC}"
    sudo iptables -t mangle -F MARK_PACKETS
    sudo iptables -t mangle -D OUTPUT -j MARK_PACKETS
    sudo iptables -t mangle -X MARK_PACKETS
    echo "${YEL}Previous iptables configuration removed.${NC}"
fi

if sudo tc qdisc show dev eth0 | grep -q "qdisc netem 10:" > /dev/null 2>&1
then
    echo "${YEL}Previous tc configuration found. Removing it.${NC}"
    sudo tc qdisc del dev eth0 root
    echo "${YEL}Previous tc configuration removed.${NC}"
fi

echo "${PINK}Enabling the malicious delay attack.${NC}"

sudo iptables -t mangle -N MARK_PACKETS
sudo iptables -t mangle -A OUTPUT -j MARK_PACKETS
sudo iptables -t mangle -A MARK_PACKETS -d $COOLTEE_HON_REQ_SUBNET -j MARK --set-mark 46

sudo tc qdisc add dev eth0 root handle 1: prio
sudo tc qdisc add dev eth0 parent 1:1 handle 10: netem delay $ATTACK_DELAY
sudo tc filter add dev eth0 protocol ip parent 1:0 prio 1 handle 46 fw flowid 1:1
echo "${PINK}Enabled the malicious delay attack.${NC}"