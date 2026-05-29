#ifndef PASSWORDHASH_H
#define PASSWORDHASH_H

#include <string>
#include <sstream>
#include <iomanip>   

// Функция для простого хеширования паролей (алгоритм djb2)
// Преобразует строку пароля в 16-значное шестнадцатеричное число
inline std::string simpleHash(const std::string& input) {
    unsigned long hash = 5381;  // Начальное значение для алгоритма djb2

    // Основной цикл хеширования: hash = hash * 33 + c
    for (char c : input) {
        hash = ((hash << 5) + hash) + static_cast<unsigned char>(c);
        // (hash << 5) + hash  эквивалентно hash * 33
    }

    // Преобразуем число в шестнадцатеричную строку
    std::stringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << hash;
    return ss.str();  // Возвращаем 16-значную hex-строку
}

#endif