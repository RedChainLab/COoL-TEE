#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cassert>
#include <map>
#include <thread>

#include "Node.h"
#include "Enclave_u.h"

#define crypto_aead_aes256gcm_ABYTES    16U

#define NODE_MGR "[Node Mgr]> "

std::map<int, Node*> Node::nodes;
const char* Node::ENCLAVE_FILE = "node/enclave.signed.so";

/* Global EID shared by multiple threads */
sgx_enclave_id_t global_eid = 0;

typedef struct _sgx_errlist_t {
    sgx_status_t err;
    const char *msg;
    const char *sug; /* Suggestion */
} sgx_errlist_t;

/* Error code returned by sgx_create_enclave */
static sgx_errlist_t sgx_errlist[] = {
    {
        SGX_ERROR_UNEXPECTED,
        "Unexpected error occurred.",
        NULL
    },
    {
        SGX_ERROR_INVALID_PARAMETER,
        "Invalid parameter.",
        NULL
    },
    {
        SGX_ERROR_OUT_OF_MEMORY,
        "Out of memory.",
        NULL
    },
    {
        SGX_ERROR_ENCLAVE_LOST,
        "Power transition occurred.",
        "Please refer to the sample \"PowerTransition\" for details."
    },
    {
        SGX_ERROR_INVALID_ENCLAVE,
        "Invalid enclave image.",
        NULL
    },
    {
        SGX_ERROR_INVALID_ENCLAVE_ID,
        "Invalid enclave identification.",
        NULL
    },
    {
        SGX_ERROR_INVALID_SIGNATURE,
        "Invalid enclave signature.",
        NULL
    },
    {
        SGX_ERROR_OUT_OF_EPC,
        "Out of EPC memory.",
        NULL
    },
    {
        SGX_ERROR_NO_DEVICE,
        "Invalid SGX device.",
        "Please make sure SGX module is enabled in the BIOS, and install SGX driver afterwards."
    },
    {
        SGX_ERROR_MEMORY_MAP_CONFLICT,
        "Memory map conflicted.",
        NULL
    },
    {
        SGX_ERROR_INVALID_METADATA,
        "Invalid enclave metadata.",
        NULL
    },
    {
        SGX_ERROR_DEVICE_BUSY,
        "SGX device was busy.",
        NULL
    },
    {
        SGX_ERROR_INVALID_VERSION,
        "Enclave version was invalid.",
        NULL
    },
    {
        SGX_ERROR_INVALID_ATTRIBUTE,
        "Enclave was not authorized.",
        NULL
    },
    {
        SGX_ERROR_ENCLAVE_FILE_ACCESS,
        "Can't open enclave file.",
        NULL
    },
    {
        SGX_ERROR_MEMORY_MAP_FAILURE,
        "Failed to reserve memory for the enclave.",
        NULL
    },
    {
        SGX_ERROR_ENCLAVE_CRASHED,
        "The enclave is crashed.",
        NULL
    }
};

/* Check error conditions for loading enclave */
void print_error_message(sgx_status_t ret)
{
    size_t idx = 0;
    size_t ttl = sizeof sgx_errlist/sizeof sgx_errlist[0];

    for (idx = 0; idx < ttl; idx++) {
        if(ret == sgx_errlist[idx].err) {
            if(NULL != sgx_errlist[idx].sug)
                printf("Info: %s\r\n", sgx_errlist[idx].sug);
            printf("Error: %s\r\n", sgx_errlist[idx].msg);
            break;
        }
    }

    if (idx == ttl)
        printf("Error: Unexpected error occurred: %x.\r\n", ret);
}

inline void set_affinity(int core_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);

    pid_t pid = getpid();

    int result = sched_setaffinity(pid, sizeof(cpu_set_t), &cpuset);
    if (result != 0) {
        std::cerr << "Error setting thread affinity: " << strerror(result) << std::endl;
    }
}

inline void set_thread_affinity(int core_id) {
    pthread_t t = pthread_self();
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);

    int s = pthread_setaffinity_np(t, sizeof(cpu_set_t), &cpuset);
    if (s != 0) {
        std::cerr << "Error setting thread affinity: " << strerror(s) << std::endl;
    }
    std::cout<<"[utrst]> Core "<<sched_getcpu()<<std::endl;
}

static int loop_recvfrom(int enclave_id, uint16_t port)
{
    printf("[utrst]> ENode listen starting...\r\n");
    int retval = 0;
    sgx_status_t ret = ecall_loop_recvfrom(enclave_id, &retval, port);
    if (ret != SGX_SUCCESS) 
    {
        print_error_message(ret);
    }
    printf("[utrst]> ENode listen finished.\r\n");
    return retval;
}

static int loop_refresh(int enclave_id, uint16_t port)
{
    printf("[utrst]> ENode refresh starting...\r\n");
    int retval = 0;
    sgx_status_t ret = ecall_refresh(enclave_id, &retval, port);
    if (ret != SGX_SUCCESS) 
    {
        print_error_message(ret);
    }
    printf("[utrst]> ENode refresh finished.\r\n");
    return retval;
}

static int loop_untaint_trigger(int enclave_id, uint16_t port)
{
    printf("[utrst]> ENode untaint trigger starting...\r\n");
    int retval = 0;
    sgx_status_t ret = ecall_untaint_trigger(enclave_id, &retval, port);
    if (ret != SGX_SUCCESS) 
    {
        print_error_message(ret);
    }
    printf("[utrst]> ENode untaint trigger finished.\r\n");
    return retval;
}

static int monitor(int enclave_id, uint16_t port, int core_id)
{
    printf("[utrst]> Starting ENode monitoring...\r\n");
    set_thread_affinity(core_id);
    int retval = 0;
    sgx_status_t ret = ecall_monitor(enclave_id, &retval, port);
    if (ret != SGX_SUCCESS) 
    {
        print_error_message(ret);
    }
    printf("[utrst]> ENode monitor finished.\r\n");
    return retval;
}

Node::Node(uint16_t _port, int _core_rdTSC) : stop(false), port(_port), core_rdTSC(_core_rdTSC), enclave_id(0)
{
    /* Configuration for Switchless SGX */
    sgx_uswitchless_config_t us_config = SGX_USWITCHLESS_CONFIG_INITIALIZER;
    us_config.num_uworkers = 12;
    us_config.num_tworkers = 24;

    if (initialize_enclave(&us_config) < 0) 
    {
        std::cerr << getPrefix() << "Error: enclave initialization failed" << std::endl;
        enclave_id = 0;
    } 
    else 
    {
        std::cout << getPrefix() << "SGX enclave initialized: " << enclave_id << std::endl;
    }
    int retval = 0;
    sgx_status_t ret = ecall_init(enclave_id, &retval, port);
    if (ret != SGX_SUCCESS) 
    {
        print_error_message(ret);
    }
    std::cout << getPrefix() << "Node initialized" << std::endl;

//  Comment if not using Triad
//  /*
    threads.emplace_back(loop_recvfrom, enclave_id, port);
    threads.emplace_back(loop_untaint_trigger, enclave_id, port);
    threads.emplace_back(loop_refresh, enclave_id, port);
    threads.emplace_back(monitor, enclave_id, port, core_rdTSC);
//  */
}

Node::~Node() 
{
    std::cout << getPrefix() << "Destroying node instance..." << std::endl;
    std::cout << getPrefix() << "Signalling to stop..." << std::endl;
    int retvalue = 0;
    stop = true;
    sgx_status_t ret = ecall_stop(enclave_id, &retvalue, port);
    if (ret != SGX_SUCCESS) 
    {
        print_error_message(ret);
    }
    std::cout << getPrefix() << "ENode stopped." << std::endl;
    std::cout << getPrefix() << "Joining threads..." << std::endl;
    for(auto& thread : this->threads)
    {
        std::cout << getPrefix() << "Joining thread?" << std::endl;
        if(thread.joinable())
        {
            thread.join();
            std::cout << getPrefix() << "Thread joined." << std::endl;
        }
        else
        {
            std::cout << getPrefix() << "Thread not joinable." << std::endl;
        }
    }
    std::cout << getPrefix() << "Threads joined." << std::endl;
    std::cout << getPrefix() << "Destroying ENode..." << std::endl;
    ret = ecall_destroy(enclave_id, &retvalue, port);
    if (ret != SGX_SUCCESS) 
    {
        print_error_message(ret);
    }
    std::cout << getPrefix() << "ENode destroyed." << std::endl;
    if (enclave_id != 0) 
    {
        sgx_destroy_enclave(enclave_id);
        std::cout << getPrefix()  << "SGX enclave destroyed: " << enclave_id << std::endl;
    }
}

/* Initialize the enclave:
 *   Call sgx_create_enclave to initialize an enclave instance
 */
int Node::initialize_enclave(const sgx_uswitchless_config_t* us_config)
{
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;

    /* Call sgx_create_enclave to initialize an enclave instance */
    /* Debug Support: set 2nd parameter to 1 */

    const void* enclave_ex_p[32] = { 0 };

    enclave_ex_p[SGX_CREATE_ENCLAVE_EX_SWITCHLESS_BIT_IDX] = (const void*)us_config;

    ret = sgx_create_enclave_ex(ENCLAVE_FILE, SGX_DEBUG_FLAG, NULL, NULL, &enclave_id, NULL, SGX_CREATE_ENCLAVE_EX_SWITCHLESS, enclave_ex_p);
    if (ret != SGX_SUCCESS) {
        print_error_message(ret);
        return -1;
    }

    return 0;
}

Node* Node::get_instance(uint16_t _port, int _core_rdTSC)
{
    std::cout << NODE_MGR << "Trying to create node with port "<< _port << "..." << std::endl;
    if (nodes.find(_port) == nodes.end())
    {
        std::cout << NODE_MGR << "Creating node instance..." << std::endl;
        nodes[_port] = new Node(_port, _core_rdTSC);
        std::cout << NODE_MGR << "Node instance created: " << nodes[_port] << std::endl;
    }
    else
    {
        std::cout << NODE_MGR << "Node instance exists: " << nodes[_port] << std::endl;
    }
    return nodes[_port];
}

void Node::destroy_instance(uint16_t _port)
{
    std::cout << NODE_MGR << "Trying to destroy node with port " << _port << "..." << std::endl;
    if (!nodes.empty() && nodes.find(_port) != nodes.end())
    {
        std::cout << NODE_MGR << "Destroying node instance: " << nodes[_port] << std::endl;
        delete nodes[_port];
        nodes.erase(_port);
        std::cout << NODE_MGR << "Node instance destroyed." << std::endl;
    }
    else
    {
        std::cout << NODE_MGR << "Node instance does not exist." << std::endl;
    }
}

std::string Node::getPrefix()
{
    return "[Node "+std::to_string(port)+"]> ";
}

timespec Node::get_timestamp()
{
    timespec ts;
    int retval = 0;
    sgx_status_t ret = ecall_get_timestamp(enclave_id, &retval, port, &ts);
    if (ret != SGX_SUCCESS) 
    {
        print_error_message(ret);
    }
    return ts;
}

void Node::print_timestamp()
{
    timespec ts_node=this->get_timestamp();
    timespec ts_ref;
    timespec_get(&ts_ref, TIME_UTC);
    if(!stop)
    {
        char buff[100];
        strftime(buff, sizeof buff, "%D %T", gmtime(&(ts_ref.tv_sec)));
        printf("%sRef. Time: %s.%09ld UTC\n", getPrefix().c_str(), buff, ts_ref.tv_nsec);
        strftime(buff, sizeof buff, "%D %T", gmtime(&(ts_node.tv_sec)));
        printf("%sNode Time: %s.%09ld UTC\n", getPrefix().c_str(), buff, ts_node.tv_nsec);
    }

}

void Node::poll_timestamp(int count)
{
    threads.emplace_back([this, count, stop=std::ref(this->stop)](){
        for(int i=0; (count<0 || i<count) && !stop; i+=(count>=0)?1:0)
        {
            print_timestamp();
            usleep(1000000);
        }
    });
}

int Node::add_sibling(const std::string& hostname, uint16_t _port)
{
    int retval = 0;
    sgx_status_t ret = ecall_add_sibling(enclave_id, &retval, port, hostname.c_str(), _port);
    if (ret != SGX_SUCCESS) 
    {
        print_error_message(ret);
    }
    return retval;
}