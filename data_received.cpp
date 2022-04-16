#include "main.h"

#define C_ALL(X) cbegin(X), cend(X)

//extern server echo_server;

struct m_data
{
    string command;
    vector<string> args{};
};

// Тут можно ограничиться только вектром lobbies, но так будет быстрее работать, а памяти ест не намного больше
extern vector<string> lobby_names;
extern vector<lobby_info> lobbies;

const regex regular_ex("^(\\w+)|(\\[([\\w\\{\\}\\s\\:\\.\\,\\'\\\"]+)\\])");

static string create_lobby_returns();
static int find_lobby_index(string str);

void _data_received(server* s, connection_hdl hdl, message_ptr msg)
{
    std::cout << "on_message called with hdl: " << hdl.lock().get()
              << " and message: " << msg->get_payload()
              << std::endl;

    m_data msg_data;
    opcode_value opcode = msg->get_opcode();
    string message = msg->get_payload();

    const vector<smatch> matches{
        sregex_iterator{C_ALL(message), regular_ex},
        sregex_iterator{}
    };

    bool num = true;
    for (auto match : matches)
    {
        if (num)
        {
            msg_data.command = match[0];
            num = false;
        }
        else       
            msg_data.args.push_back(match[3]);   
    }

    if (msg_data.command == "create_lobby")
    {
        create_lobby(msg_data.args[0], msg_data.args[1], msg_data.args[2], msg_data.args[3], hdl, opcode);
    }
    else if (msg_data.command == "find_lobby")
    {
        if (msg_data.args[0] == VERSION)
            send(hdl, opcode, create_lobby_returns());
        else 
            send(hdl, opcode, "_old_version_");
    }
    else if (msg_data.command == "join_to_lobby")
    {
        join_to_lobby(msg_data.args[0], msg_data.args[1], msg_data.args[2], msg_data.args[3], hdl, opcode);
    }
    else if (msg_data.command == "change_stat")
    {
        change_stat(msg_data.args[0], msg_data.args[1], msg_data.args[2], hdl, opcode);
    }   
}

void create_lobby(string lobby_name, string password, string difficulty, string game_version, connection_hdl hdl, opcode_value opcode)
{
    lobby_info temp_info;
    temp_info.lobby_name = lobby_name;
    temp_info.password = password;
    temp_info.difficulty = difficulty;
    temp_info.game_version = game_version;
    temp_info.host_hdl = hdl;
    temp_info.host_opcode = opcode;

    
    if (find(lobby_names.begin(), lobby_names.end(), lobby_name) == lobby_names.end() and game_version == VERSION)
    {
        lobbies.push_back(temp_info);
        lobby_names.push_back(lobby_name);
        send(hdl, opcode, "_lobby_created_");
    }
    else if (find(lobby_names.begin(), lobby_names.end(), lobby_name) != lobby_names.end())
        send(hdl, opcode, "_lobby_name_exist_");
    else
        send(hdl, opcode, "_old_version_");
}

void join_to_lobby(string lobby_name, string password, string user_name, string client, connection_hdl hdl, opcode_value opcode)
{
    int lobby_index = find_lobby_index(lobby_name);

    if (lobby_index == -1)
        send(hdl, opcode, "_lobby_not_exist_");
    else
    {
        if (client == "host")
        {
            lobbies[lobby_index].host_name = user_name;
            send(hdl, opcode, "_connected_to_lobby_");
        }
        else if (client == "client")
        {
            if (password == lobbies[lobby_index].password)
            {
                if (lobbies[lobby_index].client_name == "")
                {
                    lobbies[lobby_index].client_hdl = hdl;
                    lobbies[lobby_index].client_opcode = opcode;
                    lobbies[lobby_index].client_name = user_name;
                    
                    send(hdl, opcode, "_connected_to_lobby_ ["+lobbies[lobby_index].host_name+"]");
                    send(lobbies[lobby_index].host_hdl, lobbies[lobby_index].host_opcode, "_client_connected_to_lobby_ ["+user_name+"]");           
                    lobbies[lobby_index].status = "hide";         
                }
                else
                    send(hdl, opcode, "_lobby_overflow_");
            }
            else
                send(hdl, opcode, "_invalid_password_");
        }
    }
}

void change_stat(string lobby_name, string client, string status, connection_hdl hdl, opcode_value opcode)
{
    int lobby_index = find_lobby_index(lobby_name);

    if (lobby_index != -1)
    {
        if (client == "host")
            send(lobbies[lobby_index].client_hdl, lobbies[lobby_index].client_opcode, "_host_"+status+"_");
        else if (client == "client")
            send(lobbies[lobby_index].host_hdl, lobbies[lobby_index].host_opcode, "_client_"+status+"_");
    }
}


static string create_lobby_returns()
{
    string result = "[";

    unsigned short num = 1;
    for (lobby_info info: lobbies)
    {
        result += "{\"lobby_name\":\"" + info.lobby_name + "\","\
                  +"\"difficulty\":\"" + info.difficulty + "\","\
                  +"\"status\":\"" + info.status + "\"}";

        if (num != lobbies.size())
        {
            result += ",";
            ++num;
        } 
    }
    
    result += "]";
    cout << result << endl;

    return result;
}

static int find_lobby_index(string str)
{
    unsigned short num = 0;
    for (lobby_info lobby: lobbies)
    {
        if (lobby.lobby_name == str)
            return num;
        else
            ++num;
    }
    return -1;
}