
#include <asio.hpp>
#include <chrono>
#include <iostream>

using asio::ip::udp;
using namespace std::chrono;

const int N = 1000000;


long long run_udp_test(asio::io_context& io_context, const std::string& host , unsigned short port) {
    udp::socket socket(io_context, udp::v4());
    udp::endpoint receiver_endpoint(asio::ip::address::from_string(host), port);
    std::array<int, 1> inbuffer;
    std::array<int, 1> outbuffer;
    long long total_time_ns = 0;

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; ++i) {
        
        
        // 1. SEND  
        socket.send_to(asio::buffer(outbuffer), receiver_endpoint);
        
        // 2. RECEIVE ACK
        socket.receive_from(asio::buffer(inbuffer), receiver_endpoint);
        
        
    }
    auto end = high_resolution_clock::now();
    total_time_ns = duration_cast<nanoseconds>(end - start).count();
    return total_time_ns;
}

int main(){

    asio::io_context io;

    long long total_time_ns = run_udp_test(io, "127.0.0.1", 8080);
    std::cout << "Time Taken: "<< total_time_ns << " ns" <<std::endl;
    std::cout << "Time Taken per full round: "<< total_time_ns / N << " ns" <<std::endl;

}