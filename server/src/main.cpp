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
    // Получаем ВСЕХ пользователей из БД
    std::vector<std::string> allUsers = Database::getInstance().getAllUsers();

    std::string list = "/users ";
    for (const auto& name : allUsers) {
        // Можно добавить отметку, кто онлайн
        bool isOnline = false;
        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            isOnline = (userToSocket.find(name) != userToSocket.end());
        }

        if (isOnline) {
            list += name + "* ";  // * значит онлайн
        }
        else {
            list += name + " ";   // без звёздочки — офлайн
        }
    }

    // Рассылаем всем клиентам
    std::lock_guard<std::mutex> lock(clientsMutex);
    for (auto& [sock, name] : clients) {
        send(sock, list.c_str(), list.length(), 0);
        send(sock, "\n", 1, 0);
    }
}

void handleClient(int clientSocket) {
    char buffer[4096];

    // Получаем сообщение с логином/паролем
    int n = recv(clientSocket, buffer, 4095, 0);
    if (n <= 0) return;
    buffer[n] = '\0';

    // Ожидаем формат: "username|password"
    std::string data(buffer);
    size_t sep = data.find('|');
    if (sep == std::string::npos) {
        send(clientSocket, "/error Invalid login format\n", 28, 0);
        close(clientSocket);
        return;
    }

    std::string username = data.substr(0, sep);
    std::string password = data.substr(sep + 1);

    // Убираем \n
    while (!username.empty() && (username.back() == '\n' || username.back() == '\r')) {
        username.pop_back();
    }
    while (!password.empty() && (password.back() == '\n' || password.back() == '\r')) {
        password.pop_back();
    }

    std::string passwordHash = simpleHash(password);

    // Проверяем, существует ли пользователь
    if (Database::getInstance().userExists(username)) {
        // Логин
        if (!Database::getInstance().loginUser(username, passwordHash)) {
            send(clientSocket, "/error Invalid password\n", 24, 0);
            close(clientSocket);
            return;
        }

        Logger::getInstance().log("USER_LOGIN: " + username);
    }
    else {
        // Регистрация
        if (!Database::getInstance().registerUser(username, passwordHash)) {
            send(clientSocket, "/error Registration failed\n", 28, 0);
            close(clientSocket);
            return;
        }
        Logger::getInstance().log("USER_REGISTER: " + username);
    }

    // Регистрация
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        clients[clientSocket] = username;
        userToSocket[username] = clientSocket;
    }

    send(clientSocket, "/welcome Login successful\n", 26, 0);
    broadcastUserList();

    // Обработка сообщений
    while (true) {
        n = recv(clientSocket, buffer, 4095, 0);
        if (n <= 0) break;
        buffer[n] = '\0';

        std::string data(buffer);

        // Убираем символ новой строки
        while (!data.empty() && (data.back() == '\n' || data.back() == '\r')) {
            data.pop_back();
        }

        if (data.empty()) continue;

        std::cout << "[DEBUG] Received: " << data << std::endl;

        // Обработка команды /list
        if (data == "/list") {
            std::string list = "/users ";
            std::lock_guard<std::mutex> lock(clientsMutex);
            for (auto& [sock, name] : clients) {
                if (sock != clientSocket) {
                    list += name + " ";
                }
            }
            send(clientSocket, list.c_str(), list.length(), 0);
            send(clientSocket, "\n", 1, 0);
            continue;
        }

        // Парсим сообщение
        Message msg = Message::deserialize(data);

        std::cout << "[DEBUG] Message type: " << msg.type << std::endl;

        if (msg.type == "msg") {
            if (msg.from == msg.to) {
                send(clientSocket, "/error Cannot send message to yourself\n", 40, 0);
                continue;  // не сохраняем и не отправляем
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
                    std::string forwarded = msg.serialize();
                    send(targetSock, forwarded.c_str(), forwarded.length(), 0);
                    send(targetSock, "\n", 1, 0);
                    delivered = true;
                    Logger::getInstance().log("MSG: " + msg.from + " -> " + msg.to + ": " + msg.body);
                    std::cout << "[DEBUG] Message delivered to " << msg.to << std::endl;
                }
            }

            // Отправляем подтверждение отправителю (только если пользователь офлайн)
            if (!delivered) {
                send(clientSocket, "/error User offline, but message saved\n", 40, 0);
            }
            else {
                send(clientSocket, "/delivered\n", 11, 0);
            }
        }
        else if (msg.type == "history") {
            int limit = std::stoi(msg.body);
            std::cout << "[DEBUG] History request for " << msg.from << " with " << msg.to << ", limit=" << limit << std::endl;

            std::vector<std::string> history = Database::getInstance().getHistory(msg.from, msg.to, limit);

            std::cout << "[DEBUG] Found " << history.size() << " messages" << std::endl;

            // Отправляем историю клиенту
            send(
                clientSocket,
                "/history_start\n",
                strlen("/history_start\n"),
                0
            );
            for (const auto& line : history) {

                std::string out =
                    "/history_line " + line + "\n";

                send(clientSocket,
                    out.c_str(),
                    out.size(),
                    0);
            }
            send(
                clientSocket,
                "/history_end\n",
                strlen("/history_end\n"),
                0
            );
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