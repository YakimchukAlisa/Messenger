#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <iostream>
#include <ctime>
#include <mutex>

class Logger {
private:
    std::ofstream file;
    std::mutex mtx;
    static Logger* instance;

    Logger() {
        file.open("messenger.log", std::ios::app);
    }

public:
    static Logger& getInstance() {
        if (!instance) instance = new Logger();
        return *instance;
    }

    void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(mtx);
        time_t now = time(0);
        std::string timestamp = ctime(&now);
        timestamp.pop_back();

        std::string entry = "[" + timestamp + "] " + message;
        std::cout << entry << std::endl;
        if (file.is_open()) {
            file << entry << std::endl;
            file.flush();
        }
    }
};

Logger* Logger::instance = nullptr;

#endif