#include "main.h"

server echo_server;

int main() 
{
    try {
        // Set logging settings
        echo_server.clear_access_channels(websocketpp::log::alevel::all);
        echo_server.clear_error_channels(websocketpp::log::alevel::all);

        // Инициализирует ASIO
        echo_server.init_asio();

        // Обработчики событий, вызывают другие функции
        echo_server.set_open_handler(bind(&_connected, ::_1));                              // Когда пользователь присоеденился (Не используется)
        echo_server.set_message_handler(bind(&_data_received, &echo_server, ::_1, ::_2));   // Когда получено сообщение от пользователя
        echo_server.set_close_handler(bind(&_disconnected, ::_1));                          // Когда пользователь отключился

        // Слушет порт указаный в main.h
        echo_server.listen(PORT);

        // Запускает серверный цикл приёма
        echo_server.start_accept();

        // Запускает io_service ASIO цикл
        echo_server.run();
    } catch (websocketpp::exception const & e) {
        std::cout << e.what() << std::endl;
    } catch (...) {
        std::cout << "other exception" << std::endl;
    }
}

// Функция отправки, более сокращённая
void send(websocketpp::connection_hdl hdl, opcode_value opcode, string message)
{
    websocketpp::lib::error_code ec;
    echo_server.send(hdl, message, opcode, ec);
}
