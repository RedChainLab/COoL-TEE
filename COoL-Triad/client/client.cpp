#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include <cstring>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <chrono>
#include <set>
#include <algorithm>

#include "json.hpp"

using namespace json;

std::mutex print_mutex;

struct Stats {
    int sentMsgCount;
};

struct ServerInfo {
    std::string hostname;
    int port;
    double weight;
    Stats stats;
};

struct ClientInfo {
    std::string name;
    double x1;
    int kReqs;
    double kErr;
    double kD;
    int windowSize;
    int updateRatiosCount;
    std::vector<double> providerAvgLatency;
    std::vector<double> providerPrevAvgRelLatency;
    std::vector<double> r3;
    std::vector<double> providerRatios;
    std::vector<std::vector<double>> providerLatencies;
};

std::string read_file_to_string(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

std::string resolve_hostname(const std::string& hostname) {
    struct addrinfo hints, *res;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;  // IPv4
    hints.ai_socktype = SOCK_STREAM;
    
    if (getaddrinfo(hostname.c_str(), NULL, &hints, &res) != 0) {
        throw std::runtime_error("Failed to resolve hostname: " + hostname);
    }
    
    struct sockaddr_in* addr = (struct sockaddr_in*)res->ai_addr;
    std::string ip = inet_ntoa(addr->sin_addr);
    freeaddrinfo(res);
    return ip;
}

std::vector<ServerInfo> read_server_info(const std::string& filename) {
    std::string file_content = read_file_to_string(filename);
    JSON network_json = JSON::Load(file_content);
    std::vector<ServerInfo> servers;

    for (auto& server : network_json["servers"].ArrayRange()) {
        ServerInfo info;
        info.hostname = server["hostname"].ToString();
        info.port = server["port"].ToInt();
        info.weight = server["weight"].ToFloat();
        servers.push_back(info);
    }

    return servers;
}

ClientInfo read_client_info(const std::string& filename, const int client_idx) {
    std::string file_content = read_file_to_string(filename);
    JSON network_json = JSON::Load(file_content);
    ClientInfo client;

    client.name = network_json["clients"][client_idx]["name"].ToString();

    return client;
}

void updateRatios(ClientInfo& client)
{
    int nSPs = client.providerRatios.size();

    auto itBegin = client.providerAvgLatency.size()>=client.windowSize?client.providerAvgLatency.end()-client.windowSize+1:client.providerAvgLatency.begin();
    double sum = std::accumulate(itBegin, client.providerAvgLatency.end(), 0.);

    double Dt = sum / std::min(int(client.providerAvgLatency.size()),client.windowSize);

    for(int i=0; i<nSPs; i++)
    {
        double Di = client.providerAvgLatency[i];
        double Drel = (Dt-Di)/Dt;
        double dRi=client.kErr*Drel;

        static bool firstRun=true;
        if(firstRun)
        {
            firstRun=false;
        }
        else
        {
            double oldDrel=client.providerPrevAvgRelLatency[i];
            double dRiD=Drel-oldDrel;

            dRi+=client.kD*dRiD;
        }
        client.providerPrevAvgRelLatency[i]=Drel;
        client.r3[i]=std::clamp(client.r3[i]+dRi,0.,1.);
    }

    double sumR3=std::accumulate(client.r3.begin(), client.r3.end(), 0.);
    
    for(int i=0; i<nSPs; i++)
    {
        client.r3[i]=client.r3[i]/sumR3;
        
        client.providerRatios[i]=client.x1*(1/double(nSPs))+(1-client.x1)*client.r3[i];
    }
}

void select_servers(ClientInfo& client, std::vector<int>& selected_servers, std::vector<ServerInfo>& servers) {
    std::vector<double> cumulative_weights;
    double total_weight = 0.0;

    for (const double serverRatio : client.providerRatios) {
        total_weight += serverRatio;
        cumulative_weights.push_back(total_weight);
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dist(0.0, total_weight);

    std::set<int> selected_indices;

    for (int i = 0; i < client.kReqs; ++i) {
        double random_weight = dist(gen);
        auto it = std::upper_bound(cumulative_weights.begin(), cumulative_weights.end(), random_weight);
        int index = std::distance(cumulative_weights.begin(), it);
        
        // Check if the server has already been selected
        if (selected_indices.find(index) != selected_indices.end()) {
            --i; // Retry selecting a server
            continue;
        }
        
        selected_servers[i]=index;
        selected_indices.insert(index);
    }
}

ssize_t readAll(int sock, std::vector<char> &buffer) {
    const size_t CHUNK_SIZE = 512;
    char chunk[CHUNK_SIZE];
    ssize_t totalRead = 0;

    while (true) {
        ssize_t bytesRead = read(sock, chunk, sizeof(chunk) - 1);
        if (bytesRead < 0) {
            std::cerr << "Failed to read from socket\n";
            return -1;
        } else if (bytesRead == 0) {
            // End of data
            break;
        } else {
            totalRead += bytesRead;
            buffer.insert(buffer.end(), chunk, chunk + bytesRead);
        }
    }

    return totalRead;
}

void send_request(ClientInfo& client, ServerInfo& server_info, JSON& request, int server_idx)
{
    int cliSockDes, readStatus;
    struct sockaddr_in serAddr;
    socklen_t serAddrLen;

    //create a socket
    if ((cliSockDes = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation error...\n");
        exit(-1);
    }

    //server socket address
    serAddr.sin_family = AF_INET;
    serAddr.sin_port = htons(server_info.port);
    serAddr.sin_addr.s_addr = inet_addr(server_info.hostname.c_str());

    std::string request_str = request.dump();
    std::vector<char> buffer;
    auto start_time = std::chrono::steady_clock::now();
    if (sendto(cliSockDes, request_str.c_str(), request_str.length(), 0, (struct sockaddr*)&serAddr, sizeof(serAddr)) < 0) {
        perror("sending error...\n");
        close(cliSockDes);
        exit(-1);
    }
    serAddrLen = sizeof(serAddr);
    char buff[1024] = {0};
    readStatus = recvfrom(cliSockDes, buff, 1024, 0, (struct sockaddr*)&serAddr, &serAddrLen);
    if (readStatus < 0) {
        perror("reading error...\n");
        close(cliSockDes);
        exit(-1);
    }
    //std::cout << "Server: " << buff << std::endl;
    auto end_time = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end_time - start_time;
    {
        std::lock_guard<std::mutex> lock(print_mutex);
        std::cout << client.name << ";" << request["ID"].ToInt() << ";"<< server_idx << ";" << start_time.time_since_epoch().count() << ";" << elapsed_seconds.count()<<std::endl;
#ifdef COOL
        int nbDataPoints=client.providerLatencies[server_idx].size();
        client.providerAvgLatency[server_idx]=(client.providerAvgLatency[server_idx]*nbDataPoints+elapsed_seconds.count())/(nbDataPoints+1);
        client.providerLatencies[server_idx].push_back(elapsed_seconds.count());
        static int counter=0;
        if(client.updateRatiosCount>0 && (++counter)%client.updateRatiosCount==0)
        {
            updateRatios(client);
        }
#endif
    }   
    std::string response(buff);
    JSON jresp = JSON::Load(response);
    close(cliSockDes);
}

void send_requests(ClientInfo& client, std::vector<ServerInfo>& servers, const std::vector<int>& selected_servers) {
    static int rqID = 0;

    JSON request;
    request["ID"] = rqID++;
    request["keywords"] = "desearch\r"; 
    request["page"] = "1";

    std::vector<std::thread> send_threads;
    for (int server_idx : selected_servers) {
        ServerInfo& server_info = servers[server_idx];
        server_info.stats.sentMsgCount++;
        send_threads.emplace_back(send_request, std::ref(client), std::ref(server_info), std::ref(request), server_idx);
    }

    for (auto& thread : send_threads) {
        thread.join();
    }
}

int main(int argc, char* argv[]) {
    if(argc != 8) {
        std::cout << "USAGE: ./client <client-network-idx> <k-reqs> <kErr> <kD> <x1> <updateRatiosCount> <windowSize>" << std::endl;
        return 1;
    }

    std::vector<ServerInfo> servers = read_server_info("./network.json");

    ClientInfo client;
    client.name = std::to_string(std::stoi(argv[1]));
    client.kReqs = std::stoi(argv[2]);
    client.kErr = std::stod(argv[3]);
    client.kD = std::stod(argv[4]);
    client.x1 = std::stod(argv[5]);
    client.updateRatiosCount = std::stoi(argv[6]);
    client.windowSize = std::stoi(argv[7]);
    client.providerAvgLatency.resize(servers.size(), 0.);
    client.providerPrevAvgRelLatency.resize(servers.size(), 0.);
    client.r3.resize(servers.size(), 1./servers.size());
    client.providerRatios.resize(servers.size(), 1./servers.size());
    client.providerLatencies.resize(servers.size(), std::vector<double>());
    
    std::vector<int> selectedServers(client.kReqs);
    try {
        for (;;) {
            select_servers(client, selectedServers, servers);
            send_requests(client, servers, selectedServers);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}