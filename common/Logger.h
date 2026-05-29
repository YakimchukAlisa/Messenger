#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <iostream>
#include <ctime>
#include <mutex>

// Класс-одиночка (Singleton) для логирования событий
// Записывает сообщения в файл messenger.log и дублирует в консоль
class Logger {
private:
    std::ofstream file;      // Файловый поток для записи логов
    std::mutex mtx;          // Мьютекс для потокобезопасной записи
    static Logger* instance; // Статический указатель для Singleton

    // Приватный конструктор - открывает файл лога в режиме добавления
    Logger() {
        file.open("messenger.log", std::ios::app);  // app = append (добавление в конец)
    }

public:
    // Получение экземпляра Singleton
    static Logger& getInstance() {
        if (!instance) {
            instance = new Logger();
        }
        return *instance;
    }

    // Запись сообщения в лог
    void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(mtx);  // Блокируем мьютекс для потокобезопасности

        time_t now = time(0);                   // Получаем текущее время
        std::string timestamp = ctime(&now);    // Преобразуем в строку
        timestamp.pop_back();                   // Убираем символ новой строки в конце

        std::string entry = "[" + timestamp + "] " + message;
        std::cout << entry << std::endl;        // Выводим в консоль

        if (file.is_open()) {
            file << entry << std::endl;         // Записываем в файл
            file.flush();                       // Принудительно сбрасываем буфер
        }
    }
};

// Инициализация статического члена класса (обязательно вне класса)
Logger* Logger::instance = nullptr;

#endif