#include <unistd.h>

#include "App/Prov.h"

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <port> <core_rdTSC> <triad_port>" << std::endl;
        return -1;
    }

    uint16_t port = atoi(argv[1]);
    int core_rdTSC = atoi(argv[2]);
    uint16_t prov_port = atoi(argv[3]);
    Prov* node = Prov::get_instance(port, core_rdTSC, 6250, prov_port);
    Prov* node2 = Prov::get_instance(port+1, core_rdTSC+1, 6250, prov_port+1);
    Prov* node3 = Prov::get_instance(port+2, core_rdTSC+2, 6250, prov_port+2);

    usleep(10000);
    node->add_sibling("127.0.0.1", port+1);
    node->add_sibling("127.0.0.1", port+2);
    node2->add_sibling("127.0.0.1", port+2);

    std::cout << "<Enter anything to continue>"<< std::endl;
    std::string msg;
    std::cin >> msg;

    // node->poll_timestamp(-1);
    // node2->poll_timestamp(-1);
    // node3->poll_timestamp(-1);

    std::cout << "<Enter anything to continue>"<< std::endl;
    std::cin >> msg;

    Prov::destroy_instance(prov_port);
    Prov::destroy_instance(prov_port+1);
    Prov::destroy_instance(prov_port+2);
    return 0;
}