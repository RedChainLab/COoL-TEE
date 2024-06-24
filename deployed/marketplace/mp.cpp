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

using namespace json;

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
        message["SEND"]=std::chrono::system_clock::now().time_since_epoch().count();

        std::unique_lock<std::mutex> lock(queue_mutex);
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

int main() {
    const std::string multicast_address = "239.255.0.1";
    const unsigned short multicast_port = 30001;
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
    std::this_thread::sleep_for(std::chrono::seconds(10));
    done = true;
    cv.notify_all();

    // Wait for all threads to finish
    producer_thread.join();
    for (auto& thread : consumer_threads) {
        thread.join();
    }

    return 0;
}
