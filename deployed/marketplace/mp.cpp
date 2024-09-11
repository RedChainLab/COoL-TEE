#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <random>
#include <chrono>
#include <atomic>
#include <vector>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include "json.hpp"
#include "config.h"

using namespace json;

int asset;
std::mutex queue_mutex;
std::condition_variable cv;
std::queue<JSON> message_queue;
std::atomic<bool> done(false);

double generate_exponential_wait_time(double lambda) {
    std::random_device rd;
    std::mt19937 generator(rd());
    std::exponential_distribution<> distribution(lambda);
    return distribution(generator);
}

void producer(double lambda) {
    int message_id = 0;
    while (!done) {
        std::this_thread::sleep_for(std::chrono::duration<double>(generate_exponential_wait_time(lambda)));

        JSON message;

        std::unique_lock<std::mutex> lock(queue_mutex);
        message["asset"]=asset++;
        message_queue.push(message);
        cv.notify_all();
    }
}

void consumer(int id, const std::string& multicast_address, unsigned short multicast_port) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(multicast_port);
    addr.sin_addr.s_addr = inet_addr(multicast_address.c_str());

    while (!done || !message_queue.empty()) {
        std::unique_lock<std::mutex> lock(queue_mutex);
        cv.wait(lock, [] { return !message_queue.empty() || done; });

        if (!message_queue.empty()) {
            JSON message = message_queue.front();
            message_queue.pop();
            lock.unlock();

            std::string message_str = message.dump();
            if (sendto(sockfd, message_str.c_str(), message_str.size(), 0, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
                perror("sendto");
            } else {
                std::cout << "Consumer " << id << " sent message: " << message_str << std::endl;
            }
        }
    }

    close(sockfd);
}

void listenToClient(int client_socket)
{
    while(true)
    {
        try {
            char data[512];
            std::cout << "Waiting for client message: " << std::endl;
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

            std::cout << "Received request from client: " << request.dump() << std::endl;

        } catch (const std::exception& e) {
            std::cerr << "Exception in handle_client: " << e.what() << "\n";
            // Close client socket on exception
            close(client_socket);
        }
    }
}

int listenToClients(int port)
{
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

    std::vector<std::thread> client_conns_threads;

    for(;;)
    {
        try
        {
            std::cout << "Waiting for client connection on port " << port << std::endl;
            if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) 
            {
                perror("accept");
                close(server_fd);
                exit(EXIT_FAILURE);
            }
            std::cout << "Accepted client connection on port " << port << std::endl;
            client_conns_threads.emplace_back(listenToClient, client_socket);

        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
            return 1;
        }
    }

    for(auto& thread : client_conns_threads)
    {
        thread.join();
    }

    return 0;
}

void handle_asset_gen()
{
    double lambda = 1.0; // Rate parameter for the exponential distribution
    int consumer_count = 3; // Number of consumer threads

    // Start producer thread
    std::thread producer_thread(producer, lambda);

    // Start consumer threads
    std::vector<std::thread> consumer_threads;
    for (int i = 0; i < consumer_count; ++i) {
        consumer_threads.emplace_back(consumer, i, multicast_address, multicast_port);
    }

    // Let the system run for some time
    //std::this_thread::sleep_for(std::chrono::seconds(10));
    //done = true;
    //cv.notify_all();

    // Wait for all threads to finish
    producer_thread.join();
    for (auto& thread : consumer_threads) {
        thread.join();
    }
}

int main() {
    std::thread asset_gen_thread(handle_asset_gen);
    std::thread client_conn_thread(listenToClients, 12340);
    asset_gen_thread.join();
    client_conn_thread.join();
    return 0;
}
