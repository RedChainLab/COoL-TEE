#ifndef NODE_H
#define NODE_H

#include <iostream>
#include <map>
#include <thread>
#include <vector>

#include <sgx_urts.h>
#include <sgx_uswitchless.h>

class Node {
public:
    static Node* get_instance(uint16_t port, int core_rdTSC);
    static void destroy_instance(uint16_t port);
    timespec get_timestamp();
    void print_timestamp();
    void poll_timestamp(int count);
    int add_sibling(const std::string& hostname, uint16_t port);
    bool stop;
protected:
    uint16_t port;
    int core_rdTSC;
    std::map<std::pair<std::string, uint16_t>, int> siblings;
    std::vector<std::thread> threads;

    Node(uint16_t _port, int _core_rdTSC);
    virtual ~Node();
    int initialize_enclave(const sgx_uswitchless_config_t* us_config);
    static std::map<int, Node*> nodes;
    sgx_enclave_id_t enclave_id;
    static const char* ENCLAVE_FILE;
    virtual std::string getPrefix();
};

#endif // NODE_H