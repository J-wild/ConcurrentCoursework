
#define ASIO_STANDALONE // Do not use Boost
#define _WEBSOCKETPP_NO_BOOST_ // to tell WebSocket++ not to use Boost at all. (needed for WindowsOS)
#define _WEBSOCKETPP_CPP11_THREAD_ // to use C++11 <thread> instead of Boost threads.  (needed for WindowsOS)

#include <iostream>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <chrono>

// Define the server type using the non-TLS configuration
typedef websocketpp::server<websocketpp::config::asio> server;
using namespace std::chrono;

// Define a handle for the connection
using connection_hdl = websocketpp::connection_hdl;

// Global constants
const int PORT = 9002;

/**
 * @brief Message Handler for the WebSocket Server.
 * * This function is called every time a message is received from the client.
 * It echoes the message back for RTT calculation.
 */
void on_message(server* s, connection_hdl hdl, server::message_ptr msg) {
    // 1. Check if the message is a text message and not a control frame
    if (msg->get_opcode() == websocketpp::frame::opcode::text) {
        
        try {
            // 2. Echo the payload back to the client
            s->send(hdl, msg->get_payload(), msg->get_opcode());

        } catch (websocketpp::exception const & e) {
            std::cerr << "WebSocket Send Error: " << e.what() << std::endl;
        }
    }
}

/**
 * @brief Open Handler for new connections.
 * * You could use this to record the connection open time, but we exclude it from RTT overhead.
 */
void on_open(server* s, connection_hdl hdl) {
    std::cout << "Client connected." << std::endl;
}

/**
 * @brief Close Handler for connections.
 */
void on_close(server* s, connection_hdl hdl) {
    std::cout << "Client disconnected." << std::endl;
}

int main() {
    server echo_server;

    try {
        // Set logging settings
        echo_server.set_access_channels(websocketpp::log::alevel::all);
        echo_server.clear_access_channels(websocketpp::log::alevel::frame_payload);

        // Initialize ASIO
        echo_server.init_asio();

        // Set the message callback function
        echo_server.set_message_handler(std::bind(&on_message, &echo_server, std::placeholders::_1, std::placeholders::_2));
        
        // Set other event handlers
        echo_server.set_open_handler(std::bind(&on_open, &echo_server, std::placeholders::_1));
        echo_server.set_close_handler(std::bind(&on_close, &echo_server, std::placeholders::_1));

        // Listen on the specified port
        echo_server.listen(PORT);
        std::cout << "WebSocket Server listening on port " << PORT << std::endl;

        // Start the server's ASIO io_service thread
        echo_server.start_accept();

        // Start the ASIO io_service run loop. This is the main blocking call.
        echo_server.run();

    } catch (websocketpp::exception const & e) {
        std::cerr << "WebSocket Server Error: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Other exception occurred." << std::endl;
    }

    return 0;
}