#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cassert>
#include <map>
#include <thread>

#include "Prov.h"
#include "Enclave_u.h"

#define NODE_MGR "[Prov Mgr]> "

extern void print_error_message(sgx_status_t ret);

std::map<int, Prov*> Prov::provs;

Prov* Prov::get_instance(uint16_t triad_port, int core_rdTSC, int service_time, uint16_t prov_port)
{
    std::cout << NODE_MGR << "Trying to create provider with port "<< prov_port << "..." << std::endl;
    if (provs.find(prov_port) == provs.end())
    {
        std::cout << NODE_MGR << "Creating provider instance..." << std::endl;
        provs[prov_port] = new Prov(triad_port, core_rdTSC, service_time, prov_port);
        std::cout << NODE_MGR << "Provider instance created: " << provs[prov_port] << std::endl;
    }
    else
    {
        std::cout << NODE_MGR << "Provider instance exists: " << provs[prov_port] << std::endl;
    }
    return provs[prov_port];
}

void Prov::destroy_instance(uint16_t prov_port)
{
    std::cout << NODE_MGR << "Trying to destroy provider with port " << prov_port << "..." << std::endl;
    if (!provs.empty() && provs.find(prov_port) != provs.end())
    {
        std::cout << NODE_MGR << "Destroying provider instance: " << provs[prov_port] << std::endl;
        delete provs[prov_port];
        provs.erase(prov_port);
        std::cout << NODE_MGR << "Provider instance destroyed." << std::endl;
    }
    else
    {
        std::cout << NODE_MGR << "Provider instance does not exist." << std::endl;
    }
}

static int loop_handle_rq(int enclave_id, uint16_t port)
{
    std::cout << "[Prov]> EProv handle_rq starting..." << std::endl;
    int retval = 0;
    sgx_status_t ret = ecall_loop_handle_rq_prov(enclave_id, &retval, port);
    if (ret != SGX_SUCCESS) 
    {
        print_error_message(ret);
    }
    std::cout << "[Prov]> EProv handle_rq finished." << std::endl;
    return retval;
}

static int loop_recvfrom(int enclave_id, uint16_t port)
{
    std::cout << "[Prov]> EProv listen starting..." << std::endl;
    int retval = 0;
    sgx_status_t ret = ecall_loop_recvfrom_prov(enclave_id, &retval, port);
    if (ret != SGX_SUCCESS) 
    {
        print_error_message(ret);
    }
    std::cout << "[Prov]> EProv listen finished." << std::endl;
    return retval;
}

Prov::Prov(uint16_t triad_port, int _core_rdTSC, int service_time, uint16_t _prov_port):Node(triad_port, _core_rdTSC), prov_port(_prov_port)
{
    std::cout << NODE_MGR << "Creating provider instance..." << std::endl;
    int retval = 0;
    sgx_status_t ret = ecall_init_prov(enclave_id, &retval, prov_port, service_time, triad_port);
    if (ret != SGX_SUCCESS) 
    {
        print_error_message(ret);
    }
    std::cout << NODE_MGR << "Provider instance created." << std::endl;

    threads.emplace_back(loop_recvfrom, enclave_id, prov_port);
    threads.emplace_back(loop_handle_rq, enclave_id, prov_port);
}

Prov::~Prov()
{
    std::cout << getPrefix() << "Destroying node instance..." << std::endl;
    std::cout << getPrefix() << "Signalling to stop..." << std::endl;
    int retvalue = 0;
    stop = true;
    sgx_status_t ret = ecall_stop_prov(enclave_id, &retvalue, prov_port);
    if (ret != SGX_SUCCESS) 
    {
        print_error_message(ret);
    }
    std::cout << getPrefix() << "EProv stopped." << std::endl;
    ret = ecall_stop(enclave_id, &retvalue, port);
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
    ret = ecall_destroy_prov(enclave_id, &retvalue, port);
    if (ret != SGX_SUCCESS) 
    {
        print_error_message(ret);
    }
    std::cout << getPrefix() << "EProv destroyed." << std::endl;
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

std::string Prov::getPrefix()
{
    return "[Prov "+std::to_string(port)+"]> ";
}

