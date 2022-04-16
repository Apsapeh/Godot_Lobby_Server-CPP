#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <regex>

typedef websocketpp::server<websocketpp::config::asio> server;
typedef server::message_ptr message_ptr;
typedef websocketpp::frame::opcode::value opcode_value;

using namespace std;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
using websocketpp::connection_hdl;


// Данные о сервере
const short PORT = 9080;
const string VERSION = "0.00";


// Структура с данными о лобби
struct lobby_info
{
    string lobby_name;
    string password;
    string difficulty;
    string game_version;
    string status = "show";
    // Информация о хосте
    connection_hdl host_hdl;
    opcode_value host_opcode;
    string host_name;
    // Информация о клиенте
    connection_hdl client_hdl;
    opcode_value client_opcode;
    string client_name;
};

void _connected(websocketpp::connection_hdl);
void _disconnected(websocketpp::connection_hdl);
void _data_received(server* s, connection_hdl hdl, message_ptr msg);

//
void create_lobby(string lobby_name, string password, string difficulty, string game_version, connection_hdl hdl, opcode_value opcode);
void join_to_lobby(string lobby_name, string password, string user_name, string client, connection_hdl hdl, opcode_value opcode);
void change_stat(string lobby_name, string client, string status, connection_hdl hdl, opcode_value opcode);

//
void send(websocketpp::connection_hdl hdl, opcode_value opcode, string message);