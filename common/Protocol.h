#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Структура сообщения для обмена между клиентом и сервером
// Поддерживает: аутентификацию, текстовые сообщения, ответы (reply), пересылку (forward)
struct Message {
    // Основные поля
    std::string type;              // Тип сообщения: "auth", "msg", "history", "user_list" и т.д.
    std::string from;             // Отправитель
    std::string to;               // Получатель
    std::string body;             // Текст сообщения
    std::string group;            // Название группы (для групповых чатов)

    // Списки и вложенные сообщения
    std::vector<std::string> users;      // Список пользователей (для user_list)
    std::vector<Message> messages;       // Вложенные сообщения (для истории)

    // Параметры запросов и ответов
    int limit = 0;                 // Лимит для запроса истории
    int code = 0;                  // Код ошибки (для error)
    time_t timestamp = 0;          // Временная метка сообщения

    // Идентификаторы
    std::string msg_id;            // ID сообщения (альтернативное поле)
    std::string id;                // Уникальный ID сообщения

    // Поддержка ответов (reply)
    std::string reply_to;          // ID сообщения, на которое отвечаем

    // Поддержка пересылки (forward)
    bool is_forwarded = false;     // Флаг, что сообщение переслано
    std::string original_from;     // Оригинальный отправитель
    std::string original_body;     // Оригинальный текст сообщения

    // Преобразование сообщения в JSON-строку для отправки
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

        // Поля для reply и forward
        if (!id.empty()) j["id"] = id;
        if (!reply_to.empty()) j["reply_to"] = reply_to;
        if (is_forwarded) j["is_forwarded"] = is_forwarded;
        if (!original_from.empty()) j["original_from"] = original_from;
        if (!original_body.empty()) j["original_body"] = original_body;

        return j.dump() + "\n";  // Добавляем символ новой строки как разделитель
    }

    // Преобразование JSON-строки обратно в структуру Message
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

        // Десериализация полей reply и forward
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