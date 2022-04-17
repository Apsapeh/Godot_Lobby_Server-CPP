#include "main.h"

#define C_ALL(X) cbegin(X), cend(X)

//extern server echo_server;

// Структура с получемыми данными от пользователя
struct m_data
{
    string command;         // Название команды
    vector<string> args{};  // Аргумкенты к команде
};

// Объявление внешних списков лобби
extern vector<string> lobby_names;  // Список с названиями лобби (Чтобы быстрее искало, памяти ест на 5% больше, чем если без этого списка)
extern vector<lobby_info> lobbies;  // Список с информацией о всех лобби

// Регулярное выражнение для парсинга сообщений от пользователя
const regex regular_ex("^(\\w+)|(\\[([\\w\\{\\}\\s\\:\\.\\,\\'\\\"]+)\\])");

// Прототипы функций
static string create_lobby_returns();       // Возвращет все лобби в виде строки для отправки пользователю
static int find_lobby_index(string str);    // Возвращяет индекс лобби по названию


///////////////////////////////////////////////////////////////// Основные функции ///////////////////////////////////////////////////////

// Функция обработки получаемых сообщений
void _data_received(server* s, connection_hdl hdl, message_ptr msg)
{
    m_data msg_data;                            // Тут и так понятно
    opcode_value opcode = msg->get_opcode();    // Сохраняет opcode, опять же, для скорости
    string message = msg->get_payload();        // Сохраняет сообщение строкой, опять же, для скорости

    // Парсит сообщение и сохрание в вектор
    const vector<smatch> matches{
        sregex_iterator{C_ALL(message), regular_ex},
        sregex_iterator{}
    };

    // Обработка отпарсеного сообщения 
    bool num = true;    // Определяет команда или аргумент
    for (auto match : matches)
    {
        if (num)
        {
            msg_data.command = match[0]; // Сохраняет как команду
            num = false;
        }
        else       
            msg_data.args.push_back(match[3]);   // Сохраняет как аргумент
    }

    // Обработка команд (Без комментариев, все вызывает в том порядке, в котором они идут)
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

// Функция создания лобби 
void create_lobby(string lobby_name, string password, string difficulty, string game_version, connection_hdl hdl, opcode_value opcode)
{
    // Установка информации о создаваемом лобби
    lobby_info temp_info;
    temp_info.lobby_name    = lobby_name;
    temp_info.password      = password;
    temp_info.difficulty    = difficulty;
    temp_info.game_version  = game_version;
    temp_info.host_hdl      = hdl;
    temp_info.host_opcode   = opcode;

    
    // Если лобби не существует и версия игры равна версии сервера
    if (find(lobby_names.begin(), lobby_names.end(), lobby_name) == lobby_names.end() and game_version == VERSION)
    {
        lobbies.push_back(temp_info);           // Сохраняет информацию о лобби
        lobby_names.push_back(lobby_name);      // Сохраняет название лобби
        send(hdl, opcode, "_lobby_created_");   // Лобби создано
    }
    // Если лобби с таким именем уже существует
    else if (find(lobby_names.begin(), lobby_names.end(), lobby_name) != lobby_names.end())
        send(hdl, opcode, "_lobby_name_exist_");
    // Если версия игры устарела
    else
        send(hdl, opcode, "_old_version_");
}

// Фунция подключения к лобби как клиента, так и хоста
void join_to_lobby(string lobby_name, string password, string user_name, string client, connection_hdl hdl, opcode_value opcode)
{
    int lobby_index = find_lobby_index(lobby_name); // Индекс лобби по имени

    // Если лобби не существет
    if (lobby_index == -1)
        send(hdl, opcode, "_lobby_not_exist_");
    // Если всё нормально
    else
    {
        // Если подключаемый к лобби - хост
        if (client == "host")
        {
            lobbies[lobby_index].host_name = user_name; // Сохраняет имя хоста
            send(hdl, opcode, "_connected_to_lobby_");  // Подключение успешно
        }
        
        // Если подключаемый к лобби - клиент
        else if (client == "client")
        {
            // Если пароль от лобби равен паролю полученному от клента
            if (password == lobbies[lobby_index].password)
            {   
                // Если в лобби находится только хост
                if (lobbies[lobby_index].client_name == "")
                {   
                    // Сохраняет данные о клиенте
                    lobbies[lobby_index].client_hdl     = hdl;
                    lobbies[lobby_index].client_opcode  = opcode;
                    lobbies[lobby_index].client_name    = user_name;
                    
                    send(hdl, opcode, "_connected_to_lobby_ ["+lobbies[lobby_index].host_name+"]"); // Оповещет клиента о удачном подключении к хосту
                    send(lobbies[lobby_index].host_hdl, lobbies[lobby_index].host_opcode, "_client_connected_to_lobby_ ["+user_name+"]"); // Клиент подключён к хосту
                    lobbies[lobby_index].status = "hide"; // Делает лобби скрытым с эрана 
                }
                // Если лобби уже заполнено
                else
                    send(hdl, opcode, "_lobby_overflow_");
            }
            // Иначе говорит клиенту, что пароль неверный
            else
                send(hdl, opcode, "_invalid_password_");
        }
    }
}

// Меняет статус готовности клиента/хоста в лобби
void change_stat(string lobby_name, string client, string status, connection_hdl hdl, opcode_value opcode)
{
    int lobby_index = find_lobby_index(lobby_name); // Индекс лобби по имени

    // Если такое лобби существует
    if (lobby_index != -1)
    {
        // Если пользователь - хост
        if (client == "host")
            send(lobbies[lobby_index].client_hdl, lobbies[lobby_index].client_opcode, "_host_"+status+"_");
        // Если пользователь - клиент
        else if (client == "client")
            send(lobbies[lobby_index].host_hdl, lobbies[lobby_index].host_opcode, "_client_"+status+"_");
    }
}


/////////////////////////////////////////////////////// Дополнительные функции ////////////////////////////////////////////////////

// Возвращет все лобби в виде строки для отправки пользователю (Только название лобби, сложность и статтус)
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


// Возвращет индекс лобби по имени
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
