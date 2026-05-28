#include <iostream>
#include <thread>
#include <map>
#include <algorithm>
#include <vector>
#include <mutex>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include "../../common/Protocol.h"
#include "../../common/Logger.h"
#include "../../common/Database.h"
#include "../../common/PasswordHash.h"

#define PORT 7777

std::map<int, std::string> clients;
std::map<std::string, int> userToSocket;
std::mutex clientsMutex;

void broadcastUserList() {
    std::vector<std::string> allUsers = Database::getInstance().getAllUsers();
    
    std::cout << "[DEBUG] Broadcasting user list, users count: " << allUsers.size() << std::endl;
    
    Message userListMsg;
    userListMsg.type = "user_list";
    
    for (const auto& name : allUsers) {
        bool isOnline = false;
        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            isOnline = (userToSocket.find(name) != userToSocket.end());
        }
        
        if (isOnline) {
            userListMsg.users.push_back(name + "*");
        } else {
            userListMsg.users.push_back(name);
        }
    }
    
    std::lock_guard<std::mutex> lock(clientsMutex);
    std::string serialized = userListMsg.serialize() + "\n";
    
    std::cout << "[DEBUG] Sending user list: " << serialized << std::endl;
    
    for (auto& [sock, name] : clients) {
        send(sock, serialized.c_str(), serialized.length(), 0);
    }
}

void handleClient(int clientSocket) {
    char buffer[4096];
    std::string pendingData;  // Для накопления данных

    // Получаем первое сообщение (аутентификация)
    int n = recv(clientSocket, buffer, 4095, 0);
    if (n <= 0) return;
    buffer[n] = '\0';

    std::string data(buffer);

    // Убираем \n
    while (!data.empty() && (data.back() == '\n' || data.back() == '\r')) {
        data.pop_back();
    }

    std::cout << "[DEBUG] Received auth: " << data << std::endl;

    // Парсим JSON
    Message msg;
    try {
        msg = Message::deserialize(data);
    }
    catch (...) {
        send(clientSocket, "{\"type\":\"error\",\"code\":400,\"body\":\"Invalid JSON\"}\n", 50, 0);
        close(clientSocket);
        return;
    }

    // Проверяем тип сообщения (должен быть "auth")
    if (msg.type != "auth") {
        send(clientSocket, "{\"type\":\"error\",\"code\":400,\"body\":\"First message must be auth\"}\n", 60, 0);
        close(clientSocket);
        return;
    }

    std::string username = msg.from;
    std::string password = msg.body;

    // Убираем \n на всякий случай
    while (!username.empty() && (username.back() == '\n' || username.back() == '\r')) {
        username.pop_back();
    }
    while (!password.empty() && (password.back() == '\n' || password.back() == '\r')) {
        password.pop_back();
    }

    std::string passwordHash = simpleHash(password);

    // Проверяем, существует ли пользователь
    bool loginSuccess = false;
    if (Database::getInstance().userExists(username)) {
        // Логин
        if (Database::getInstance().loginUser(username, passwordHash)) {
            loginSuccess = true;
            Logger::getInstance().log("USER_LOGIN: " + username);
        }
    }
    else {
        // Регистрация
        if (Database::getInstance().registerUser(username, passwordHash)) {
            loginSuccess = true;
            Logger::getInstance().log("USER_REGISTER: " + username);
        }
    }

    if (!loginSuccess) {
        Message error;
        error.type = "error";
        error.code = 401;
        error.body = "Invalid username or password";
        send(clientSocket, error.serialize().c_str(), error.serialize().size(), 0);
        close(clientSocket);
        return;
    }

    // Регистрируем пользователя как онлайн
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        clients[clientSocket] = username;
        userToSocket[username] = clientSocket;
    }

    // Отправляем подтверждение
    Message welcome;
    welcome.type = "auth_ok";
    welcome.body = "Welcome, " + username + "!";
    send(clientSocket, welcome.serialize().c_str(), welcome.serialize().size(), 0);

    // Рассылаем обновлённый список пользователей
    broadcastUserList();

    // Обработка сообщений
    while (true) {
        n = recv(clientSocket, buffer, 4095, 0);
        if (n <= 0) break;
        buffer[n] = '\0';

        pendingData += buffer;

        // Разбираем сообщения по строкам (каждое сообщение заканчивается \n)
        size_t pos;
        while ((pos = pendingData.find('\n')) != std::string::npos) {
            std::string line = pendingData.substr(0, pos);
            pendingData.erase(0, pos + 1);

            // Убираем \r
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }

            if (line.empty()) continue;

            std::cout << "[DEBUG] Received: " << line << std::endl;

            Message msg;
            try {
                msg = Message::deserialize(line);
            }
            catch (...) {
                Message error;
                error.type = "error";
                error.code = 400;
                error.body = "Invalid JSON";
                send(clientSocket, error.serialize().c_str(), error.serialize().size(), 0);
                continue;
            }

            std::cout << "[DEBUG] Message type: " << msg.type << std::endl;

            if (msg.type == "msg") {
                if (msg.from != username) {
                    Message error;
                    error.type = "error";
                    error.code = 403;
                    error.body = "Message from field doesn't match your username";
                    send(clientSocket, error.serialize().c_str(), error.serialize().size(), 0);
                    continue;
                }

                if (msg.from == msg.to) {
                    Message error;
                    error.type = "error";
                    error.code = 400;
                    error.body = "Cannot send message to yourself";
                    send(clientSocket, error.serialize().c_str(), error.serialize().size(), 0);
                    continue;
                }

                // Сохраняем в базу данных
                Database::getInstance().saveMessage(msg.from, msg.to, msg.body, msg.timestamp);
                std::cout << "[DB] Saved message from " << msg.from << " to " << msg.to << std::endl;

                // Отправляем получателю, если он онлайн
                bool delivered = false;
                {
                    std::lock_guard<std::mutex> lock(clientsMutex);
                    if (userToSocket.find(msg.to) != userToSocket.end()) {
                        int targetSock = userToSocket[msg.to];
                        Message forward;
                        forward.type = "msg";
                        forward.from = msg.from;
                        forward.to = msg.to;
                        forward.body = msg.body;
                        forward.timestamp = msg.timestamp;
                        std::string serialized = forward.serialize();
                        send(targetSock, serialized.c_str(), serialized.size(), 0);
                        delivered = true;
                        Logger::getInstance().log("MSG: " + msg.from + " -> " + msg.to + ": " + msg.body);
                        std::cout << "[DEBUG] Message delivered to " << msg.to << std::endl;
                    }
                }

                // Отправляем подтверждение отправителю
                Message deliveryStatus;
                if (!delivered) {
                    deliveryStatus.type = "error";
                    deliveryStatus.code = 404;
                    deliveryStatus.body = "User offline, but message saved";
                }
                else {
                    deliveryStatus.type = "delivered";
                    deliveryStatus.to = msg.to;
                }
                send(clientSocket, deliveryStatus.serialize().c_str(), deliveryStatus.serialize().size(), 0);
            }
            else if (msg.type == "history") {
                int limit = msg.limit > 0 ? msg.limit : 50;
                std::cout << "[DEBUG] History request for " << msg.from << " with " << msg.to << ", limit=" << limit << std::endl;

                std::vector<std::string> history = Database::getInstance().getHistory(msg.from, msg.to, limit);

                std::cout << "[DEBUG] Found " << history.size() << " messages" << std::endl;

                // Отправляем историю клиенту
                std::string historyStart = "{\"type\":\"history_start\"}\n";
                send(clientSocket, historyStart.c_str(), historyStart.size(), 0);

                for (const auto& line : history) {
                    // Экранируем кавычки в строке
                    std::string escaped = line;
                    size_t pos = 0;
                    while ((pos = escaped.find('"', pos)) != std::string::npos) {
                        escaped.replace(pos, 1, "\\\"");
                        pos += 2;
                    }

                    std::string out = "{\"type\":\"history_line\",\"body\":\"" + escaped + "\"}\n";
                    send(clientSocket, out.c_str(), out.size(), 0);
                }

                std::string historyEnd = "{\"type\":\"history_end\"}\n";
                send(clientSocket, historyEnd.c_str(), historyEnd.size(), 0);
            }
            else if (msg.type == "list_users") {
                std::vector<std::string> allUsers = Database::getInstance().getAllUsers();

                Message userList;
                userList.type = "user_list";

                for (const auto& name : allUsers) {
                    bool isOnline = false;
                    {
                        std::lock_guard<std::mutex> lock(clientsMutex);
                        isOnline = (userToSocket.find(name) != userToSocket.end());
                    }

                    if (isOnline && name != username) {
                        userList.users.push_back(name + "*");
                    }
                    else if (name != username) {
                        userList.users.push_back(name);
                    }
                }

                send(clientSocket, userList.serialize().c_str(), userList.serialize().size(), 0);
            }
        }
    }

    // Отключение
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        clients.erase(clientSocket);
        userToSocket.erase(username);
    }

    Logger::getInstance().log("USER_LOGOUT: " + username);
    broadcastUserList();
    close(clientSocket);
}

int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Bind failed!" << std::endl;
        return 1;
    }

    if (listen(serverSocket, 10) < 0) {
        std::cerr << "Listen failed!" << std::endl;
        return 1;
    }

    Logger::getInstance().log("SERVER STARTED on port " + std::to_string(PORT));
    std::cout << "Server running on port " << PORT << std::endl;

    while (true) {
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket < 0) {
            std::cerr << "Accept failed!" << std::endl;
            continue;
        }
        std::cout << "[DEBUG] New client connected" << std::endl;
        std::thread t(handleClient, clientSocket);
        t.detach();
    }

    return 0;
}