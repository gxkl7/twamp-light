//constructor
TwampLightSender::TwampLightSender(const std::string& ip, uint16_t port): reflector_ip(ip), reflector_port(port) {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }
}

void TwampLightSender::run(int num_packets = 3, int interval_ms = 10, int timeout_interval_ms = 1000) {
    sockaddr_in reflector_addr{};
    reflector_addr.sin_family = AF_INET;
    reflector_addr.sin_port = htons(reflector_port);
    reflector_addr.sin_addr.s_addr = inet_addr(reflector_ip.c_str());

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dis(0, UINT32_MAX);
    std::vector<double> rtts_ms;
    
    for (int i = 0; i < num_packets; ++i) {
        uint32_t seq = dis(gen);
        uint64_t send_time = get_current_time_ns();
        TwampLightPacket pkt(seq, send_time, 0);
        std::vector<uint8_t> buffer = pkt.serialize();
        ssize_t sent = sendto(sockfd, buffer.data(), buffer.size(), 0, (sockaddr*)&reflector_addr, sizeof(reflector_addr));
        if (sent < 0) {
            std::cerr <<"Error Sending" << std::endl;
            continue;
        }
        // Wait for response
        struct timeval tv;
        tv.tv_sec = timeout_interval_ms / 1000;  // seconds
        tv.tv_usec = (timeout_interval_ms % 1000) * 1000; // microseconds
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
        // Parsing the response
        uint8_t recv_buffer[20];
        sockaddr_in from_addr{};
        socklen_t from_len = sizeof(from_addr);
        ssize_t len = recvfrom(sockfd, recv_buffer, sizeof(recv_buffer), 0, (sockaddr*)&from_addr, &from_len);
        uint64_t recv_time = get_current_time_ns();
        if (len < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::cerr << "No response received (timeout)" << std::endl;
                continue;
            } else {
                perror("recvfrom error");
                continue;
            }
        }
        if (len < 20) {
            std::cerr << "Malformed TWAMP packet: undersized (received " << len << " bytes, expected 20)" << std::endl;
            // Handle as malformed packet (increment malformed counter, log, etc.)
            continue;
        }
        // Deserialize and compute RTT
        TwampLightPacket resp = TwampLightPacket::deserialize(recv_buffer, len);
        double rtt_ms = (recv_time - resp.sender_timestamp) / 1e6;
        std::cout << "Seq: " << resp.sequence_number << " RTT: " << rtt_ms << " ms" << std::endl;
        rtts_ms.push_back(rtt_ms);
        //Adding delay
        if (i+1 != num_packets)
            std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
        }
        double avg_rtt = rtts_ms.empty() ? 0.0 : std::accumulate(rtts_ms.begin(), rtts_ms.end(), 0.0) / rtts_ms.size();
        std::cout << "Average RTT: " << avg_rtt << " ms" << std::endl;
    close(sockfd);
    }

