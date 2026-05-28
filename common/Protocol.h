#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string>
#include <vector>
#include <nlohmann/json.hpp>  // Нужно установить: sudo apt install nlohmann-json3-dev

using json = nlohmann::json;

struct Message {
    std::string type;     // "msg", "auth", "history", etc.
    std::string from;
    std::string to;
    std::string body;
    std::string group;
    std::vector<std::string> users;
    std::vector<Message> messages;
    int limit;
    int code;
    time_t timestamp;
    std::string msg_id;

    // Превращаем сообщение в JSON строку
    std::string serialize() const {
        json j;
        j["type"] = type;
        if (!from.empty()) j["from"] = from;
        if (!to.empty()) j["to"] = to;
        if (!body.empty()) j["body"] = body;
        if (!group.empty()) j["group"] = group;
        if (!users.empty()) j["users"] = users;
        if (limit > 0) j["limit"] = limit;
        if (code != 0) j["code"] = code;
        if (timestamp != 0) j["timestamp"] = timestamp;
        if (!msg_id.empty()) j["msg_id"] = msg_id;
        return j.dump() + "\n";
    }

    // Парсим JSON строку
    static Message deserialize(const std::string& data) {
        json j = json::parse(data);
        Message msg;
        msg.type = j.value("type", "");
        msg.from = j.value("from", "");
        msg.to = j.value("to", "");
        msg.body = j.value("body", "");
        msg.group = j.value("group", "");
        msg.limit = j.value("limit", 0);
        msg.code = j.value("code", 0);
        msg.timestamp = j.value("timestamp", 0);
        msg.msg_id = j.value("msg_id", "");
        if (j.contains("users")) {
            msg.users = j["users"].get<std::vector<std::string>>();
        }
        return msg;
    }
};

#endif