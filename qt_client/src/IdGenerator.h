#ifndef IDGENERATOR_H
#define IDGENERATOR_H

#include <string>
#include <random>

inline std::string generateMessageId() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);

    const char* hex = "0123456789abcdef";
    std::string id;
    id.reserve(32);
    for (int i = 0; i < 32; ++i) {
        id += hex[dis(gen)];
    }
    return id;
}

#endif