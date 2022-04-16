#include "main.h"

static int find_lobby_index(string client, void* hdl);

vector<string> lobby_names;
vector<lobby_info> lobbies;

void _disconnected(connection_hdl hdl)
{
    int host_index = find_lobby_index("host", hdl.lock().get());
    int client_index = find_lobby_index("client", hdl.lock().get());

    if (host_index != -1 and client_index == -1)
    {
        if (lobbies[host_index].status == "hide" and lobbies[host_index].client_name != "")
            send(lobbies[host_index].client_hdl, lobbies[host_index].client_opcode, "_lobby_closed_");

        lobbies.erase(lobbies.begin() + host_index);
        lobby_names.erase(lobby_names.begin() + host_index);

    }
    else if (host_index == -1 and client_index != -1)
    {
        connection_hdl temp_hdl;
        opcode_value temp_opcode;

        send(lobbies[client_index].host_hdl, lobbies[client_index].host_opcode, "_client_disconnected_");
        lobbies[client_index].status = "show";
        lobbies[client_index].client_hdl = temp_hdl;
        lobbies[client_index].client_opcode = temp_opcode;
        lobbies[client_index].client_name = "";
    }
}

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
