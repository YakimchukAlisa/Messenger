#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string>
#include <sstream>
#include <ctime>

struct Message {
    std::string type;
    std::string from;
    std::string to;
    std::string body;
    time_t timestamp;

    // Превращает сообщение в строку для отправки
    std::string serialize() const {
        std::stringstream ss;
        ss << type << "|" << from << "|" << to << "|" << body << "|" << timestamp;
        return ss.str();
    }

    // Преобразует строку обратно в сообщение
    static Message deserialize(const std::string& data) {
        Message msg;
        std::stringstream ss(data);
        std::getline(ss, msg.type, '|');
        std::getline(ss, msg.from, '|');
        std::getline(ss, msg.to, '|');
        std::getline(ss, msg.body, '|');
        ss >> msg.timestamp;
        return msg;
    }
};

#endif