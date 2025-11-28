
#include <asio.hpp>
#include <iostream>
#include <array>

using asio::ip::udp;

void udp_server(asio::io_context& io_context, unsigned short port) {
    udp::socket socket(io_context, udp::endpoint(udp::v4(), port));
    std::cout << "UDP Client is ready to recieve data on port 8080" << std::endl; 
    udp::endpoint remote_endpoint;
    std::array<int, 1> inbuffer;
    

    while (true) {
        // 1. Receive data
        size_t len = socket.receive_from(asio::buffer(inbuffer), remote_endpoint);
        

        // 2. Immediately send ACK back to the sender
        socket.send_to(asio::buffer(inbuffer), remote_endpoint);
    }
}

int main(){
    asio::io_context io;
    udp_server(io,8080);
}

