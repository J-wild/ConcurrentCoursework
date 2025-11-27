#define ASIO_STANDALONE
#include <asio.hpp>
#include <iostream>
#include <vector>
#include <string>

using udp = asio::ip::udp;
using namespace std::chrono;
int main() {
    try {
        asio::io_context io;
        udp::socket socket(io, udp::endpoint(udp::v4(), 8080));
        std::cout << "UDP Client is ready to recieve data on port 8080" << std::endl;

        std::array<char, 1024> buffer; 
        udp::endpoint sender_endpoint;

        while (true) {

            std::size_t bytes_received = socket.receive_from( asio::buffer(buffer), sender_endpoint );

            std::string message(buffer.data(), bytes_received);
            std::cout << message;
         
    
            
        }
        }
        catch (std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
    }
    return 0;
}