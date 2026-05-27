#ifndef PASSWORDHASH_H
#define PASSWORDHASH_H

#include <string>
#include <sstream>
#include <iomanip>   

inline std::string simpleHash(const std::string& input) {
    unsigned long hash = 5381;
    for (char c : input) {
        hash = ((hash << 5) + hash) + static_cast<unsigned char>(c);
    }

    std::stringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << hash;
    return ss.str();
}

#endif