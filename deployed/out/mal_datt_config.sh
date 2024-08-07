COOLTEE_MAL_REQ_SUBNET="10.0.4.0/24"
ATTACK_DELAY="50ms"

RED='\033[0;31m'
YEL='\033[1;33m'
PINK='\033[1;35m'
NC='\033[0m' # No Color

for arg
do
    case $arg in
        --help|-h)
            echo -e "${PINK}Usage: $0 [{--help|-h} | [--subnet=<IP_MASK>] [--delay=<DELAY>] | --rm]${NC}"
            echo -e "  --help, -h          Display this help message."
            echo -e "  --subnet=<IP_MASK>  The mask of the subnet to which the malicious delay attack should be applied. Default: ${COOLTEE_MAL_REQ_SUBNET}"
            echo -e "  --delay=<DELAY>     The delay to be applied to the packets. Default: ${ATTACK_DELAY}"
            echo -e "  --rm                Remove a previous malicious delay attack configuration."
            echo -e "  <no arguments>      Apply the malicious delay attack with the default configuration."
            exit 0
    esac
done

for arg
do
    case $arg in
        --rm)
            echo -e "${PINK}Disabling the malicious delay attack${NC}"
            sudo iptables -t mangle -F MARK_PACKETS
            sudo iptables -t mangle -D OUTPUT -j MARK_PACKETS
            sudo iptables -t mangle -X MARK_PACKETS

            sudo tc qdisc del dev eth0 root
            echo -e "${PINK}Disabled the malicious delay attack${NC}"
            exit 0
            ;;
    esac
done

for arg
do
    case $arg in
        --subnet=*)
            COOLTEE_MAL_REQ_SUBNET="${arg#*=}"
            ;;
        --delay=*)
            ATTACK_DELAY="${arg#*=}"
            ;;
    esac
done
echo -e "${PINK}Malicious delay attack configuration:${NC}"
echo -e "COOLTEE_MAL_REQ_SUBNET: $COOLTEE_MAL_REQ_SUBNET"
echo -e "ATTACK_DELAY: $ATTACK_DELAY"

echo -e "${PINK}Checking previous configuration.${NC}"
if sudo iptables --table mangle --check OUTPUT -j MARK_PACKETS > /dev/null 2>&1
then
    echo -e "${YEL}Previous iptables configuration found. Removing it.${NC}"
    sudo iptables -t mangle -F MARK_PACKETS
    sudo iptables -t mangle -D OUTPUT -j MARK_PACKETS
    sudo iptables -t mangle -X MARK_PACKETS
    echo -e "${YEL}Previous iptables configuration removed.${NC}"
fi

if sudo tc qdisc show dev eth0 | grep -q "qdisc netem 10:" > /dev/null 2>&1
then
    echo -e "${YEL}Previous tc configuration found. Removing it.${NC}"
    sudo tc qdisc del dev eth0 root
    echo -e "${YEL}Previous tc configuration removed.${NC}"
fi

echo -e "${PINK}Enabling the malicious delay attack.${NC}"

sudo iptables -t mangle -N MARK_PACKETS
sudo iptables -t mangle -A OUTPUT -j MARK_PACKETS
sudo iptables -t mangle -A MARK_PACKETS -d $COOLTEE_MAL_REQ_SUBNET -j RETURN
sudo iptables -t mangle -A MARK_PACKETS -j MARK --set-mark 46

sudo tc qdisc add dev eth0 root handle 1: prio
sudo tc qdisc add dev eth0 parent 1:1 handle 10: netem delay $ATTACK_DELAY
sudo tc filter add dev eth0 protocol ip parent 1:0 prio 1 handle 46 fw flowid 1:1
echo -e "${PINK}Enabled the malicious delay attack.${NC}"