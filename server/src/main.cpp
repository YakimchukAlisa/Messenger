#include <iostream>
#include <thread>
#include <map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include "../../common/Protocol.h"
#include "../../common/Logger.h"

#define PORT 7777

std::map<int, std::string> clients;
std::map<std::string, int> userToSocket;
std::mutex clientsMutex;

void broadcastUserList() {
    std::string list = "/users ";
    for (auto& [sock, name] : clients) {
        list += name + " ";
    }

    for (auto& [sock, name] : clients) {
        send(sock, list.c_str(), list.length(), 0);
        send(sock, "\n", 1, 0);
    }
}

void handleClient(int clientSocket) {
    char buffer[4096];

    // Получаем имя пользователя
    int n = recv(clientSocket, buffer, 4095, 0);
    if (n <= 0) return;
    buffer[n] = '\0';
    std::string username(buffer);

    // Регистрация
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        clients[clientSocket] = username;
        userToSocket[username] = clientSocket;
    }

    Logger::getInstance().log("USER_LOGIN: " + username);
    broadcastUserList();

    // Обработка сообщений
    while (true) {
        n = recv(clientSocket, buffer, 4095, 0);
        if (n <= 0) break;
        buffer[n] = '\0';

        std::string data(buffer);
        Message msg = Message::deserialize(data);

        if (msg.type == "msg") {
            if (userToSocket.find(msg.to) != userToSocket.end()) {
                int targetSock = userToSocket[msg.to];
                std::string forwarded = msg.serialize();
                send(targetSock, forwarded.c_str(), forwarded.length(), 0);
                send(targetSock, "\n", 1, 0);
                Logger::getInstance().log("MSG: " + msg.from + " -> " + msg.to + ": " + msg.body);
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

    bind(serverSocket, (sockaddr*)&addr, sizeof(addr));
    listen(serverSocket, 10);

    Logger::getInstance().log("SERVER STARTED on port " + std::to_string(PORT));
    std::cout << "Server running on port " << PORT << std::endl;

    while (true) {
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        std::thread t(handleClient, clientSocket);
        t.detach();
    }

    return 0;
}