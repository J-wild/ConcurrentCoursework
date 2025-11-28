#define ASIO_STANDALONE // Do not use Boost
#define _WEBSOCKETPP_NO_BOOST_ // to tell WebSocket++ not to use Boost at all. (needed for WindowsOS)
#define _WEBSOCKETPP_CPP11_THREAD_ // to use C++11 <thread> instead of Boost threads.  (needed for WindowsOS)

#include <iostream>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <chrono>
#include <string>

// Define the client type using the non-TLS configuration
typedef websocketpp::client<websocketpp::config::asio_client> client;
using namespace std::chrono;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;

// --- Global Measurement Variables ---
long long total_time_ns = 0;
int messages_sent = 0;
const int N_ITERATIONS = 10000; // Total number of messages to send for RTT test
const std::string URI = "ws://localhost:9002"; // Must match the server's address
const std::string PAYLOAD(16, 'A'); // Small 16-byte message for measuring framing overhead

// Structure to hold the timing information for each message
struct MessageTimer {
    high_resolution_clock::time_point start_time;
};
std::map<std::string, MessageTimer> pending_messages;


/**
 * @brief Open Handler: Starts the timing test once the connection is established.
 */
void on_open(client* c, websocketpp::connection_hdl hdl) {
    std::cout << "Connection established. Starting " << N_ITERATIONS << " RTT tests..." << std::endl;
    
    // Start the first send operation
    try {
        c->get_con_from_hdl(hdl)->send(PAYLOAD);
        
        // Record the start time for the first message (using its content/ID as the key)
        pending_messages[PAYLOAD] = {high_resolution_clock::now()};
        messages_sent = 1;

    } catch (websocketpp::exception const & e) {
        std::cerr << "Open Handler Send Error: " << e.what() << std::endl;
    }
}

/**
 * @brief Message Handler: Calculates RTT and sends the next message.
 */
void on_message(client* c, websocketpp::connection_hdl hdl, client::message_ptr msg) {
    if (msg->get_opcode() == websocketpp::frame::opcode::text) {
        std::string received_payload = msg->get_payload();

        // 1. Calculate RTT
        auto end_time = high_resolution_clock::now();
        
        // Find the start time associated with this echoed message
        if (pending_messages.count(received_payload)) {
            auto start_time = pending_messages[received_payload].start_time;
            pending_messages.erase(received_payload); // Message handled
            
            long long rtt = duration_cast<nanoseconds>(end_time - start_time).count();
            total_time_ns += rtt;
        }

        // 2. Check if the test is complete
        if (messages_sent >= N_ITERATIONS) {
            // Test finished: Calculate average and close
            double avg_time_us = (double)total_time_ns / N_ITERATIONS / 1000.0; // ns to us
            double avg_time_ms = avg_time_us / 1000.0;

            std::cout << "\n--- RESULTS ---" << std::endl;
            std::cout << "Total Time: " << (double)total_time_ns / 1000000000.0 << " seconds" << std::endl;
            std::cout << "Avg RTT: " << avg_time_us << " microseconds" << std::endl;
            std::cout << "Avg RTT: " << avg_time_ms << " milliseconds" << std::endl;
            
            c->close(hdl, websocketpp::close::status::normal, "Test Complete");
            return;
        }

        // 3. Send the next message
        try {
            c->get_con_from_hdl(hdl)->send(PAYLOAD);
            pending_messages[PAYLOAD] = {high_resolution_clock::now()};
            messages_sent++;
        } catch (websocketpp::exception const & e) {
            std::cerr << "Message Handler Send Error: " << e.what() << std::endl;
        }
    }
}

/**
 * @brief Main function to set up and run the client.
 */
int main() {
    client ws_client;
    
    try {
        // Set logging settings
        ws_client.clear_access_channels(websocketpp::log::alevel::all);
        ws_client.clear_error_channels(websocketpp::log::elevel::all);

        // Initialize ASIO
        ws_client.init_asio();

        // Set handlers
        ws_client.set_open_handler(websocketpp::lib::bind(&on_open, &ws_client, _1));
        ws_client.set_message_handler(websocketpp::lib::bind(&on_message, &ws_client, _1, _2));

        // Create a connection
        websocketpp::lib::error_code ec;
        client::connection_ptr con = ws_client.get_connection(URI, ec);
        if (ec) {
            std::cerr << "Could not create connection: " << ec.message() << std::endl;
            return 1;
        }

        // Queue the connection
        ws_client.connect(con);

        // Start the ASIO io_service run loop. This is the main blocking call.
        ws_client.run();

    } catch (websocketpp::exception const & e) {
        std::cerr << "WebSocket Client Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
} aqqaqaqaqaqaqaqa