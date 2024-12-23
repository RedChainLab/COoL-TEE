#include "Enclave_t.h"
#include "ENode.h"
#include "EProv.h"
#include <map>

#define ENCLAVE_MGR "[ENode Mgr]> "
#define PROV_MGR "[EProv Mgr]> "
extern std::map<int /*port*/, ENode*> nodes;
extern std::map<int /*port*/, EProv*> provs;

int ecall_init(uint16_t _port)
{
    printf("%sInitializing enclave node...\r\n", ENCLAVE_MGR);
    if(nodes.find(_port) != nodes.end())
    {
        printf("%sENode already exists.\r\n", ENCLAVE_MGR);
        return SOCKET_ALREADY_EXISTS;
    }
    printf("%sENode does not exist yet. Creating...\r\n", ENCLAVE_MGR);
    nodes.emplace(_port, new ENode(_port));
    return SUCCESS;
}

int ecall_stop(uint16_t _port)
{
    printf("%sStopping ENode...\r\n", ENCLAVE_MGR);
    if(nodes.find(_port) == nodes.end())
    {
        printf("%sENode does not exist.\r\n", ENCLAVE_MGR);
        return SOCKET_INEXISTENT;
    }
    nodes[_port]->stop_tasks();
    printf("%sEnclave stopped.\r\n", ENCLAVE_MGR);
    return SUCCESS;
}

int ecall_destroy(uint16_t _port)
{
    printf("%sDestroying ENode...\r\n", ENCLAVE_MGR);
    if(nodes.find(_port) == nodes.end())
    {
        printf("%sENode does not exist.\r\n", ENCLAVE_MGR);
        return SOCKET_INEXISTENT;
    }
    delete nodes[_port];
    nodes.erase(_port);
    printf("%sENode destroyed.\r\n", ENCLAVE_MGR);
    return SUCCESS;
}

int ecall_monitor(uint16_t _port)
{
    printf("%sStarting enclave monitoring...\r\n", ENCLAVE_MGR);
    if(nodes.find(_port) == nodes.end())
    {
        printf("%sNode does not exist...\r\n", ENCLAVE_MGR);
        return SOCKET_INEXISTENT;
    }
    nodes[_port]->monitor();
    printf("%sEnclave monitoring finished.\r\n", ENCLAVE_MGR);
    return SUCCESS;
}

int ecall_loop_recvfrom(uint16_t _port)
{
    printf("%sStarting enclave recvfrom...\r\n", ENCLAVE_MGR);
    if(nodes.find(_port) == nodes.end())
    {
        printf("%sNode does not exist...\r\n", ENCLAVE_MGR);
        return SOCKET_INEXISTENT;
    }
    nodes[_port]->loop_recvfrom();
    printf("%sEnclave recvfrom finished.\r\n", ENCLAVE_MGR);
    return SUCCESS;
}

int ecall_refresh(uint16_t _port)
{
    printf("%sLaunching refresh...\r\n", ENCLAVE_MGR);
    if(nodes.find(_port) == nodes.end())
    {
        printf("%sNode does not exist...\r\n", ENCLAVE_MGR);
        return SOCKET_INEXISTENT;
    }
    nodes[_port]->refresh();
    printf("%sEnclave refresh finished.\r\n", ENCLAVE_MGR);
    return SUCCESS;
}

int ecall_untaint_trigger(uint16_t _port)
{
    printf("%sStarting untainting trigger...\r\n", ENCLAVE_MGR);
    if(nodes.find(_port) == nodes.end())
    {
        printf("%sNode does not exist...\r\n", ENCLAVE_MGR);
        return SOCKET_INEXISTENT;
    }
    nodes[_port]->untaint_trigger();
    printf("%sUntainting trigger finished.\r\n", ENCLAVE_MGR);
    return SUCCESS;
}

int ecall_add_sibling(uint16_t _port, const char* hostname, uint16_t port)
{
    printf("%sAdding sibling at %s:%d to node at %d...\r\n", ENCLAVE_MGR, hostname, port, _port);
    if(nodes.find(_port) == nodes.end())
    {
        printf("%sNode at %d does not exist...\r\n", ENCLAVE_MGR, _port);
        return SOCKET_INEXISTENT;
    }
    nodes[_port]->add_sibling(std::string(hostname), port);
    printf("%sSibling at %s:%d added to node at %d.\r\n", ENCLAVE_MGR, hostname, port, _port);
    return SUCCESS;
}

int ecall_get_timestamp(uint16_t _port, struct timespec* ts)
{
    printf("%sGetting timestamp...\r\n", ENCLAVE_MGR);
    if(nodes.find(_port) == nodes.end())
    {
        printf("%sNode does not exist...\r\n", ENCLAVE_MGR);
        return SOCKET_INEXISTENT;
    }
    *ts = nodes[_port]->get_timestamp();
    printf("%sTimestamp retrieved.\r\n", ENCLAVE_MGR);
    return SUCCESS;
}

int ecall_init_prov(uint16_t _prov_port, int _service_time, uint16_t _triad_port)
{
    printf("%sInitializing enclave provider...\r\n", PROV_MGR);
    if(nodes.find(_triad_port) == nodes.end())
    {
        printf("%sTriad node does not exist...\r\n", PROV_MGR);
        return SOCKET_INEXISTENT;
    }
    provs.emplace(_prov_port, new EProv(_prov_port, _service_time, _triad_port));
    printf("%sProvider initialized.\r\n", PROV_MGR);
    return SUCCESS;
}

int ecall_stop_prov(uint16_t _prov_port)
{
    printf("%sStopping EProv...\r\n", PROV_MGR);
    if(provs.find(_prov_port) == provs.end())
    {
        printf("%sEProv does not exist.\r\n", PROV_MGR);
        return SOCKET_INEXISTENT;
    }
    provs[_prov_port]->stop_tasks();
    printf("%sEProv stopped.\r\n", PROV_MGR);
    return SUCCESS;
}

int ecall_destroy_prov(uint16_t _prov_port)
{
    printf("%sDestroying EProv...\r\n", PROV_MGR);
    if(provs.find(_prov_port) == provs.end())
    {
        printf("%sEProv does not exist.\r\n", PROV_MGR);
        return SOCKET_INEXISTENT;
    }
    delete provs[_prov_port];
    provs.erase(_prov_port);
    printf("%sEProv destroyed.\r\n", PROV_MGR);
    return SUCCESS;
}

int ecall_loop_recvfrom_prov(uint16_t _prov_port)
{
    printf("%sStarting enclave recvfrom...\r\n", PROV_MGR);
    if(provs.find(_prov_port) == provs.end())
    {
        printf("%sEProv does not exist...\r\n", PROV_MGR);
        return SOCKET_INEXISTENT;
    }
    provs[_prov_port]->loop_recvfrom();
    printf("%sEProv recvfrom finished.\r\n", PROV_MGR);
    return SUCCESS;
}

int ecall_loop_handle_rq_prov(uint16_t _prov_port)
{
    printf("%sStarting enclave loop_handle_rq...\r\n", PROV_MGR);
    if(provs.find(_prov_port) == provs.end())
    {
        printf("%sEProv does not exist...\r\n", PROV_MGR);
        return SOCKET_INEXISTENT;
    }
    provs[_prov_port]->loop_handle_rq();
    printf("%sEProv handle_rq finished.\r\n", PROV_MGR);
    return SUCCESS;
}