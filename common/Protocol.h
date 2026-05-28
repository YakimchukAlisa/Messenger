#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct Message {
    std::string type;
    std::string from;
    std::string to;
    std::string body;
    std::string group;
    std::vector<std::string> users;
    std::vector<Message> messages;
    int limit = 0;
    int code = 0;
    time_t timestamp = 0;
    std::string msg_id;

    // Новые поля
    std::string id;
    std::string reply_to;
    bool is_forwarded = false;
    std::string original_from;
    std::string original_body;

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

        // Новые поля
        if (!id.empty()) j["id"] = id;
        if (!reply_to.empty()) j["reply_to"] = reply_to;
        if (is_forwarded) j["is_forwarded"] = is_forwarded;
        if (!original_from.empty()) j["original_from"] = original_from;
        if (!original_body.empty()) j["original_body"] = original_body;

        return j.dump() + "\n";
    }

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

        // Новые поля
        msg.id = j.value("id", "");
        msg.reply_to = j.value("reply_to", "");
        msg.is_forwarded = j.value("is_forwarded", false);
        msg.original_from = j.value("original_from", "");
        msg.original_body = j.value("original_body", "");

        if (j.contains("users")) {
            msg.users = j["users"].get<std::vector<std::string>>();
        }
        return msg;
    }
};

#endif