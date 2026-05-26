#include <iostream>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "../../common/Protocol.h"

int sock;
std::string username;
bool running = true;

void receiveMessages() {
    char buffer[4096];
    while (running) {
        int n = recv(sock, buffer, 4095, 0);
        if (n <= 0) continue;
        buffer[n] = '\0';

        std::string data(buffer);

        if (data.substr(0, 6) == "/users") {
            std::cout << "\n[Online: " << data.substr(7) << "]\n>> " << std::flush;
        }
        else {
            Message msg = Message::deserialize(data);
            std::cout << "\n[" << msg.from << "]: " << msg.body << "\n>> " << std::flush;
        }
    }
}

int main() {
    std::cout << "=== Messenger Client ===\n";
    std::cout << "Username: ";
    std::getline(std::cin, username);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(7777);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Connection failed!\n";
        return 1;
    }

    send(sock, username.c_str(), username.length(), 0);
    send(sock, "\n", 1, 0);

    std::thread receiver(receiveMessages);
    receiver.detach();

    std::cout << "Connected! Commands:\n";
    std::cout << "  /msg <user> <text> - Send message\n";
    std::cout << "  /list - Show users\n";
    std::cout << "  /quit - Exit\n>> " << std::flush;

    std::string input;
    while (std::getline(std::cin, input)) {
        if (input == "/quit") break;
        if (input == "/list") {
            send(sock, "/list\n", 6, 0);
            continue;
        }
        if (input.substr(0, 4) == "/msg") {
            size_t space1 = input.find(' ', 5);
            if (space1 != std::string::npos) {
                std::string to = input.substr(5, space1 - 5);
                std::string body = input.substr(space1 + 1);

                Message msg;
                msg.type = "msg";
                msg.from = username;
                msg.to = to;
                msg.body = body;
                msg.timestamp = time(0);

                std::string data = msg.serialize();
                std::cout << "[DEBUG] Sending: " << data << std::endl;
                send(sock, data.c_str(), data.length(), 0);
                send(sock, "\n", 1, 0);
            }
            else {
                std::cout << "[DEBUG] Wrong format! Use: /msg username text" << std::endl;
            }
        }
        std::cout << ">> " << std::flush;
    }

    running = false;
    close(sock);
    return 0;
}