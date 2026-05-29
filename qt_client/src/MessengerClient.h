#ifndef MESSENGERCLIENT_H
#define MESSENGERCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QStringList>
#include "../../common/Protocol.h"

// Клиент для сетевого взаимодействия с сервером мессенджера
// Отвечает за подключение, отправку сообщений, приём данных от сервера
class MessengerClient : public QObject
{
    Q_OBJECT

private:
    QTcpSocket* socket;          // TCP-сокет для связи с сервером
    QString username;            // Имя текущего пользователя
    bool connected;              // Флаг подключения к серверу
    QString pendingData;         // Буфер для накопления данных (т.к. данные могут приходить частями)

    void parseIncomingData(const QString& data);  // Парсинг входящих JSON-сообщений

public:
    explicit MessengerClient(QObject* parent = nullptr);
    ~MessengerClient();

    // Управление соединением
    void connectToServer(const QString& host, int port, const QString& user);  // Подключение к серверу
    void disconnectFromHost();                                                  // Отключение от сервера

    // Отправка сообщений
    void sendMessage(const QString& to, const QString& body);          // Обычное сообщение
    void sendReply(const QString& to, const QString& body, const QString& replyToId);  // Ответ на сообщение
    void forwardMessage(const QString& to, const Message& original);   // Пересылка сообщения

    // Запросы к серверу
    void requestHistory(const QString& withUser, int limit = 50);      // Запрос истории переписки
    void login(const QString& username, const QString& password);      // Аутентификация
    void requestUserList();                                            // Запрос списка пользователей

    // Геттеры
    bool isConnected() const { return connected; }
    QString getUsername() const { return username; }

signals:
    // Сигналы состояния соединения
    void connectedToServer();          // Успешное подключение к серверу
    void disconnectedFromServer();     // Отключение от сервера
    void connectionError(const QString& error);  // Ошибка соединения

    // Сигналы получения данных от сервера
    void messageReceived(const Message& msg);                 // Новое сообщение
    void userListReceived(const QStringList& users);        // Список пользователей
    void historyReceived(const QStringList& historyLines);  // История переписки
    void deliveryStatus(const QString& status);             // Статус доставки

private slots:
    // Обработчики событий сокета
    void onConnected();        // Сокет подключился
    void onDisconnected();     // Сокет отключился
    void onReadyRead();        // Пришли данные от сервера
    void onError(QAbstractSocket::SocketError error);  // Ошибка сокета
};

#endif