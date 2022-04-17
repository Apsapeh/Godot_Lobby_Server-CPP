#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <regex>

// Установка названий для типов переменных
typedef websocketpp::server<websocketpp::config::asio> server;
typedef server::message_ptr message_ptr;
typedef websocketpp::frame::opcode::value opcode_value;

using namespace std;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
using websocketpp::connection_hdl;


// Данные о сервере
const short PORT = 9080; // Прослушиваемый порт
const string VERSION = "0.00"; // Верися сервера/игры


// Структура с данными о лобби
struct lobby_info
{
    string lobby_name;      // Название лобби
    string password;        // Пароль от лобби
    string difficulty;      // Сложность лобби
    string game_version;    // Версия лобби
    string status = "show"; // Статус лобби
    
    // Информация о хосте
    connection_hdl host_hdl; 
    opcode_value host_opcode;
    string host_name;       // Имя хоста
    
    // Информация о клиенте
    connection_hdl client_hdl;
    opcode_value client_opcode;
    string client_name;     // Имя клиента
};


////////////////////////////////////////// Прототипы функций ///////////////////////////////////////////

// Функции обрабатывающие подключение/отключение/сообщение
void _connected(websocketpp::connection_hdl);                           // Подключение (Не используется)
void _disconnected(websocketpp::connection_hdl);                        // Отключение 
void _data_received(server* s, connection_hdl hdl, message_ptr msg);    // Сообщение 

// Функции из файла data_received.cpp
void create_lobby(string lobby_name, string password, string difficulty, string game_version, connection_hdl hdl, opcode_value opcode); // Функия создания лобби
void join_to_lobby(string lobby_name, string password, string user_name, string client, connection_hdl hdl, opcode_value opcode);       // Функция подключения к лобби
void change_stat(string lobby_name, string client, string status, connection_hdl hdl, opcode_value opcode);                             // Функция установки готовности

// Функция отправки (main.cpp)
void send(websocketpp::connection_hdl hdl, opcode_value opcode, string message);
