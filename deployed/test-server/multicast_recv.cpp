#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "json.hpp"
#include <unistd.h>
#include <netdb.h>

using namespace json;

void receiver(const std::string& multicast_address, unsigned short multicast_port) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(multicast_port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(sockfd);
        return;
    }

    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(multicast_address.c_str());
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        perror("setsockopt");
        close(sockfd);
        return;
    }

    char buffer[1024];
    while (true) {
        ssize_t len = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (len < 0) {
            perror("recv");
            break;
        }
        buffer[len] = '\0';

        try {
            std::string resp(buffer);
            JSON message = JSON::Load(resp);
            std::cout << "Received message: " << message.dump() << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to parse JSON: " << e.what() << std::endl;
        }
    }

    close(sockfd);
}

int main() {
    const std::string multicast_address = "239.255.0.1";
    const unsigned short multicast_port = 30001;
    while(true)
    {
        std::cout << "Waiting for messages..." << std::endl;
        receiver(multicast_address, multicast_port);
    }


    return 0;
}
