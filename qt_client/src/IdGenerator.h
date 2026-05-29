#ifndef IDGENERATOR_H
#define IDGENERATOR_H

#include <string>
#include <random>

// Генератор уникальных идентификаторов для сообщений
// Возвращает случайную 32-символьную строку из шестнадцатеричных цифр
inline std::string generateMessageId() {
    // Статические генераторы случайных чисел инициализируются один раз
    static std::random_device rd;           // Аппаратный генератор энтропии
    static std::mt19937 gen(rd());          // Вихрь Мерсенна для качественной псевдослучайности
    static std::uniform_int_distribution<> dis(0, 15);  // Равномерное распределение от 0 до 15

    const char* hex = "0123456789abcdef";   // Шестнадцатеричные цифры
    std::string id;
    id.reserve(32);                         // Резервируем память для 32 символов
    for (int i = 0; i < 32; ++i) {
        id += hex[dis(gen)];                // Добавляем случайную шестнадцатеричную цифру
    }
    return id;
}

#endif