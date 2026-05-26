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
    std::string pending;

    while (running) {

        int n = recv(sock, buffer, 4095, 0);

        if (n <= 0) {
            running = false;
            break;
        }

        buffer[n] = '\0';

        pending += buffer;

        size_t pos;

        while ((pos = pending.find('\n')) != std::string::npos) {

            std::string line = pending.substr(0, pos);

            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }

            pending.erase(0, pos + 1);

            std::cout << "[RAW] " << line << std::endl;

            if (line.rfind("/users", 0) == 0) {

                std::cout << "\n[Online: "
                    << line.substr(7)
                    << "]\n>> "
                    << std::flush;
            }
            else if (line == "/history_start") {

                std::cout << "\n=== Chat history ==="
                    << std::endl;
            }
            else if (line == "/history_end") {

                std::cout << "=== End of history ==="
                    << std::endl;

                std::cout << ">> "
                    << std::flush;
            }
            else if (line.rfind("/history_line", 0) == 0) {

                std::cout << line.substr(14)
                    << std::endl;
            }
            else {

                try {

                    Message msg =
                        Message::deserialize(line);

                    std::cout << "\n["
                        << msg.from
                        << "]: "
                        << msg.body
                        << "\n>> "
                        << std::flush;
                }
                catch (...) {

                    std::cout << "[UNKNOWN] "
                        << line
                        << std::endl;
                }
            }
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
    std::cout << "  /history <user> [limit] - Show chat history\n";
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
                send(sock, data.c_str(), data.length(), 0);
                send(sock, "\n", 1, 0);
            }
        }
        else if (input.substr(0, 8) == "/history") {
            std::string target;
            int limit = 20;

            size_t space = input.find(' ', 9);
            if (space != std::string::npos) {
                target = input.substr(9, space - 9);
                std::string limitStr = input.substr(space + 1);
                if (!limitStr.empty()) {
                    limit = std::stoi(limitStr);
                }
            }
            else {
                target = input.substr(9);
            }

            Message histMsg;
            histMsg.type = "history";
            histMsg.from = username;
            histMsg.to = target;
            histMsg.body = std::to_string(limit);
            histMsg.timestamp = time(0);

            std::string data = histMsg.serialize();
            send(sock, data.c_str(), data.length(), 0);
            send(sock, "\n", 1, 0);
        }
        std::cout << ">> " << std::flush;
    }

    running = false;
    close(sock);
    return 0;
}