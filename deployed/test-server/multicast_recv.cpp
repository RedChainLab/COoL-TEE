#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "json.hpp"
#include <unistd.h>
#include <netdb.h>
#include <thread>
#include <fstream>
#include <chrono>
#include <vector>
#include <mutex>

#include "config.h"

using namespace json;

int asset=-1;
std::mutex asset_mutex;

void receiver(const std::string& multicast_address, unsigned short multicast_port) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return;
    }

    int reuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse)) < 0) {
        perror("setsockopt SO_REUSEADDR");
        close(sockfd);
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
        perror("setsockopt IP_ADD_MEMBERSHIP");
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

            {
                std::lock_guard<std::mutex> lock(asset_mutex);
                asset = message["asset"].ToInt();
            }
        } catch (const std::exception& e) {
            std::cerr << "Failed to parse JSON: " << e.what() << std::endl;
        }
    }

    close(sockfd);
}

void receiver_loop(const std::string& multicast_address, unsigned short multicast_port) {
    while (true) {
        std::cout << "Waiting for messages..." << std::endl;
        receiver(multicast_address, multicast_port);
    }
}

struct ServerInfo {
    std::string ip;
    int port;
    double weight;
};

std::string read_file_to_string(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

ServerInfo read_server_info(const std::string& filename, const int server_idx) {
    std::string file_content = read_file_to_string(filename);
    JSON network_json = JSON::Load(file_content);

    ServerInfo info;
    info.ip = network_json["servers"][server_idx]["ip"].ToString();
    info.port = network_json["servers"][server_idx]["port"].ToInt();
    info.weight = network_json["servers"][server_idx]["weight"].ToFloat();

    return info;
}

void handle_client(int client_socket, ServerInfo& server) {
    try {
        char data[512];
        ssize_t length = read(client_socket, data, sizeof(data) - 1);
        if (length < 0) {
            std::cerr << "Error reading from socket\n";
            close(client_socket);
            return;
        }
        data[length] = '\0';  // Null-terminate the string

        // Read request
        std::string request_str(data, length);
        JSON request = JSON::Load(request_str);

        //std::cout << "Received request from client: " << request.dump() << std::endl;

        // Process request (example: echo the request)
        JSON response = request;
        {
            std::lock_guard<std::mutex> lock(asset_mutex);
            response["asset"]=asset;
        }

        // Send response
        std::string response_str = response.dump();
        usleep(6250); // Sleep for 6.25ms to simulate processing time

        ssize_t bytes_written = write(client_socket, response_str.c_str(), response_str.size());
        if (bytes_written < 0) {
            std::cerr << "Error writing response to socket\n";
        }

        // Close client socket
        close(client_socket);
    } catch (const std::exception& e) {
        std::cerr << "Exception in handle_client: " << e.what() << "\n";
        // Close client socket on exception
        close(client_socket);
    }
}

int handle_clients(int argc, char* argv[])
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <server-network-idx>\n";
        return 1;
    }
    const int SERVER_IDX = std::stoi(argv[1]);
    ServerInfo server = read_server_info("./network.json",SERVER_IDX);

    int port = server.port;
    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Bind the socket to the network address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    for(;;)
    {
        try
        {
            const int num_requests = 1000; // Number of requests to send
            auto start_time = std::chrono::steady_clock::now();

            for (int i = 0; i < num_requests; ++i) {
                if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) 
                {
                    perror("accept");
                    close(server_fd);
                    exit(EXIT_FAILURE);
                }
                handle_client(client_socket, server);
            }

            auto end_time = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed_seconds = end_time - start_time;

            double throughput = num_requests / elapsed_seconds.count();
            std::cout << "Throughput: "<< throughput << "rq/s "<< std::endl;

        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
            return 1;
        }
    }

    return 0;
}

int main(int argc, char* argv[]) {

    std::thread asset_receiver_thread(receiver_loop, multicast_address, multicast_port);
    std::thread server_thread(handle_clients, argc, argv);

    asset_receiver_thread.join();
    server_thread.join();

    return 0;
}
