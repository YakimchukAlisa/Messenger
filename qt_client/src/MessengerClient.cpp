#include "MessengerClient.h"
#include "../../common/IdGenerator.h"
#include <QDebug>

MessengerClient::MessengerClient(QObject* parent)
    : QObject(parent)
    , socket(new QTcpSocket(this))
    , connected(false)
{
    // Подключаем сигналы сокета к слотам
    connect(socket, &QTcpSocket::connected, this, &MessengerClient::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &MessengerClient::onDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &MessengerClient::onReadyRead);
    connect(socket, &QTcpSocket::errorOccurred, this, &MessengerClient::onError);
}

MessengerClient::~MessengerClient()
{
    if (connected) disconnectFromHost();
}

// Установка соединения с сервером
void MessengerClient::connectToServer(const QString& host, int port, const QString& user)
{
    username = user;
    socket->connectToHost(host, port);
}

// Принудительное отключение от сервера
void MessengerClient::disconnectFromHost()
{
    if (socket->state() == QAbstractSocket::ConnectedState) {
        socket->disconnectFromHost();
    }
    connected = false;
}

// Аутентификация на сервере (отправка имени и пароля)
void MessengerClient::login(const QString& username, const QString& password)
{
    this->username = username;

    Message authMsg;
    authMsg.type = "auth";
    authMsg.from = username.toStdString();
    authMsg.body = password.toStdString();

    std::string data = authMsg.serialize() + "\n";
    socket->write(data.c_str());
    socket->flush();

    qDebug() << "[DEBUG] Sent auth for:" << username;
}

// Отправка обычного сообщения
void MessengerClient::sendMessage(const QString& to, const QString& body)
{
    if (!connected) {
        emit connectionError("Not connected to server");
        return;
    }

    Message msg;
    msg.type = "msg";
    msg.from = username.toStdString();
    msg.to = to.toStdString();
    msg.body = body.toStdString();
    msg.timestamp = time(nullptr);
    msg.id = generateMessageId();  // Генерируем уникальный ID

    std::string data = msg.serialize() + "\n";
    socket->write(data.c_str());
    socket->flush();

    qDebug() << "[DEBUG] Sent message to:" << to << " with id:" << msg.id.c_str();
}

// Отправка ответа на конкретное сообщение (reply)
void MessengerClient::sendReply(const QString& to, const QString& body, const QString& replyToId)
{
    if (!connected) {
        emit connectionError("Not connected to server");
        return;
    }

    Message msg;
    msg.type = "msg";
    msg.from = username.toStdString();
    msg.to = to.toStdString();
    msg.body = body.toStdString();
    msg.timestamp = time(nullptr);
    msg.id = generateMessageId();
    msg.reply_to = replyToId.toStdString();  // Указываем ID исходного сообщения

    std::string data = msg.serialize() + "\n";
    socket->write(data.c_str());
    socket->flush();

    qDebug() << "[DEBUG] Sent reply to:" << to << " replying to:" << replyToId;
}

// Пересылка существующего сообщения другому пользователю
void MessengerClient::forwardMessage(const QString& to, const Message& original)
{
    if (!connected) {
        emit connectionError("Not connected to server");
        return;
    }

    Message msg;
    msg.type = "msg";
    msg.from = username.toStdString();
    msg.to = to.toStdString();
    msg.body = original.body;          // Копируем текст
    msg.timestamp = time(nullptr);
    msg.id = generateMessageId();
    msg.is_forwarded = true;           // Помечаем как пересланное
    msg.original_from = original.from; // Запоминаем оригинального отправителя
    msg.original_body = original.body; // Запоминаем оригинальный текст

    std::string data = msg.serialize() + "\n";
    socket->write(data.c_str());
    socket->flush();

    qDebug() << "[DEBUG] Forwarded message to:" << to;
}

// Запрос истории переписки с указанным пользователем
void MessengerClient::requestHistory(const QString& withUser, int limit)
{
    if (!connected) return;

    Message msg;
    msg.type = "history";
    msg.from = username.toStdString();
    msg.to = withUser.toStdString();
    msg.limit = limit;
    msg.timestamp = time(nullptr);

    std::string data = msg.serialize() + "\n";
    socket->write(data.c_str());
    socket->flush();

    qDebug() << "[DEBUG] Requested history with:" << withUser;
}

// Запрос списка всех пользователей
void MessengerClient::requestUserList()
{
    if (!connected) return;

    Message msg;
    msg.type = "list_users";
    msg.from = username.toStdString();

    std::string data = msg.serialize() + "\n";
    socket->write(data.c_str());
    socket->flush();

    qDebug() << "[DEBUG] Requested user list";
}

// Слот: подключение к серверу установлено
void MessengerClient::onConnected()
{
    connected = true;
    emit connectedToServer();
}

// Слот: соединение разорвано
void MessengerClient::onDisconnected()
{
    connected = false;
    emit disconnectedFromServer();
}

// Слот: получены данные от сервера
void MessengerClient::onReadyRead()
{
    pendingData += QString::fromUtf8(socket->readAll());

    // Разбираем сообщения по строкам (каждое сообщение заканчивается \n)
    while (pendingData.contains('\n')) {
        int newlinePos = pendingData.indexOf('\n');
        QString line = pendingData.left(newlinePos);
        pendingData = pendingData.mid(newlinePos + 1);

        if (line.endsWith('\r')) line.chop(1);

        if (!line.isEmpty()) {
            parseIncomingData(line);
        }
    }
}

// Парсинг входящих JSON-сообщений от сервера
void MessengerClient::parseIncomingData(const QString& line)
{
    qDebug() << "[DEBUG] Received:" << line;

    if (line.startsWith('{')) {
        try {
            Message msg = Message::deserialize(line.toStdString());

            if (msg.type == "auth_ok") {
                // Успешная аутентификация
                emit deliveryStatus(QString::fromStdString(msg.body));
                qDebug() << "[DEBUG] Authentication successful";
                requestUserList();  // После логина запрашиваем список пользователей
            }
            else if (msg.type == "user_list") {
                // Получен список пользователей
                QStringList users;
                for (const auto& user : msg.users) {
                    users.append(QString::fromStdString(user));
                }
                users.removeAll(username);
                users.removeAll(username + "*");
                emit userListReceived(users);
                qDebug() << "[DEBUG] User list received:" << users;
            }
            else if (msg.type == "msg") {
                // Получено новое сообщение
                emit messageReceived(msg);
                qDebug() << "[DEBUG] Message from:" << msg.from.c_str() << " id:" << msg.id.c_str();
            }
            else if (msg.type == "history") {
                // Получена история сообщений
                if (msg.messages.empty()) {
                    emit historyReceived({});
                }
                else {
                    QStringList historyLines;
                    for (const auto& histMsg : msg.messages) {
                        char timeStr[64];
                        time_t ts = histMsg.timestamp;
                        strftime(timeStr, sizeof(timeStr), "%H:%M:%S", localtime(&ts));
                        QString line = QString("[%1] %2: %3")
                            .arg(timeStr)
                            .arg(QString::fromStdString(histMsg.from))
                            .arg(QString::fromStdString(histMsg.body));
                        historyLines.append(line);
                    }
                    emit historyReceived(historyLines);
                }
            }
            else if (msg.type == "history_line") {
                // Для совместимости со старым форматом (построчная отправка)
                emit historyReceived({ QString::fromStdString(msg.body) });
            }
            else if (msg.type == "delivered") {
                // Подтверждение доставки сообщения
                emit deliveryStatus("Message delivered to " + QString::fromStdString(msg.to));
            }
            else if (msg.type == "error") {
                // Сообщение об ошибке от сервера
                emit connectionError(QString::fromStdString(msg.body));
            }
            else {
                qDebug() << "[DEBUG] Unknown message type:" << msg.type.c_str();
            }
        }
        catch (const std::exception& e) {
            qDebug() << "[DEBUG] JSON parse error:" << e.what() << "Line:" << line;
        }
    }
}

// Обработка ошибок сокета
void MessengerClient::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
        emit connectionError(socket->errorString());
}