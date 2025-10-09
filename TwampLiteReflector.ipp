#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <chrono>

//constructor
TwampLiteReflector::TwampLiteReflector(std::string ip, uint16_t port): ipaddr(ip), listen_port(port){
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ipaddr.c_str());
    addr.sin_port = htons(listen_port);
    if (bind(sockfd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(sockfd);
        exit(1);
    }
}

void TwampLiteReflector::run() {
    std::cout << "Reflector listening on " << ipaddr << ":" << listen_port << std::endl;
    while (true) {
        uint8_t buffer[20];
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        ssize_t len = recvfrom(sockfd, buffer, sizeof(buffer), 0, (sockaddr*)&client_addr, &client_len);
        if (len < 0 || len < 20) {
            std::cerr << "Malformed TWAMP packet: undersized (received " << len << " bytes, expected 20)" << std::endl;
            continue;
        }
        TwampLitePacket pkt = TwampLitePacket::deserialize(buffer, len);
        pkt.receiver_timestamp = get_current_time_ns();
        std::vector<uint8_t> send_buffer = pkt.serialize();
        sendto(sockfd, send_buffer.data(), send_buffer.size(), 0, (sockaddr*)&client_addr, client_len);
        std::cout << "Packet Served" << std::endl;
    }
}

