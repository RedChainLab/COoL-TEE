#ifndef PROV_H
#define PROV_H

#include <iostream>
#include <map>

#include <sgx_urts.h>
#include <sgx_uswitchless.h>

#include "Node.h"

class Prov: public Node {
public:
    static Prov* get_instance(uint16_t triad_port, int core_rdTSC, int service_time, uint16_t prov_port);
    static void destroy_instance(uint16_t triad_port);
    uint16_t prov_port;

protected:
    Prov(uint16_t triad_port, int core_rdTSC, int service_time, uint16_t prov_port);
    virtual ~Prov();
    std::string getPrefix() override;
    static std::map<int, Prov*> provs;
};

#endif // PROV_H