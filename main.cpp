#include "main.h"

server echo_server;

int main() 
{
    try {
        // Set logging settings
        echo_server.clear_access_channels(websocketpp::log::alevel::all);
        echo_server.clear_error_channels(websocketpp::log::alevel::all);

        // Initialize Asio
        echo_server.init_asio();

        // Register our message handler
        echo_server.set_open_handler(bind(&_connected, ::_1));
        echo_server.set_message_handler(bind(&_data_received, &echo_server, ::_1, ::_2));
        echo_server.set_close_handler(bind(&_disconnected, ::_1));

        // Listen on port 9002
        echo_server.listen(PORT);

        // Start the server accept loop
        echo_server.start_accept();

        // Start the ASIO io_service run loop
        echo_server.run();
    } catch (websocketpp::exception const & e) {
        std::cout << e.what() << std::endl;
    } catch (...) {
        std::cout << "other exception" << std::endl;
    }
}

void send(websocketpp::connection_hdl hdl, opcode_value opcode, string message)
{
    websocketpp::lib::error_code ec;
    echo_server.send(hdl, message, opcode, ec);
}