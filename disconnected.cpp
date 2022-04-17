#include "main.h"

static int find_lobby_index(string client, void* hdl);

// Доступ к внешним спискам лобби
vector<string> lobby_names; // Список с названиями лобби
vector<lobby_info> lobbies; // Список с информацией о всех лобби

// Функция обработки отключённого пользователя
void _disconnected(connection_hdl hdl)
{
    int host_index = find_lobby_index("host", hdl.lock().get());        // Поиск индекса лобби по hdl через хоста
    int client_index = find_lobby_index("client", hdl.lock().get());    // Поиск индекса лобби по hdl через клента

    // Если отключился хост
    if (host_index != -1 and client_index == -1)
    {
        // Если был клиент, то отправляет информацию об удалении лобби
        if (lobbies[host_index].status == "hide" and lobbies[host_index].client_name != "")
            send(lobbies[host_index].client_hdl, lobbies[host_index].client_opcode, "_lobby_closed_");

        lobbies.erase(lobbies.begin() + host_index);            // Удалет лобби
        lobby_names.erase(lobby_names.begin() + host_index);    // Удаляет лобби из навзаний

    }
    // Если отключился клиент
    else if (host_index == -1 and client_index != -1)
    {
        // Временные переменные 
        connection_hdl temp_hdl;
        opcode_value temp_opcode;

        send(lobbies[client_index].host_hdl, lobbies[client_index].host_opcode, "_client_disconnected_"); // Сообщение хосту о отключении клеиента

        // Изменение переменных в лобби
        lobbies[client_index].status        = "show";       // Белает лобби видимым
        lobbies[client_index].client_hdl    = temp_hdl;     // Очищает hdl
        lobbies[client_index].client_opcode = temp_opcode;  // Очищает opcode
        lobbies[client_index].client_name   = "";           // Стирает имя клиента
    }
}

// Возвращет индекс лобби по hdl
static int find_lobby_index(string client, void* hdl)
{
    unsigned short num = 0;

    if (client == "host")
    {
        for (lobby_info lobby: lobbies)
        {
            if (lobby.host_hdl.lock().get() == hdl)
                return num;
            else
                ++num;
        }
    }
    else if (client == "client")
    {
        for (lobby_info lobby: lobbies)
        {
            if (lobby.client_hdl.lock().get() == hdl)
                return num;
            else
                ++num;
        }   
    }

    return -1;
}
