#include "MessengerClient.h"
#include "../../common/IdGenerator.h"
#include <QDebug>

MessengerClient::MessengerClient(QObject* parent)
    : QObject(parent)
    , socket(new QTcpSocket(this))
    , connected(false)
{
    connect(socket, &QTcpSocket::connected, this, &MessengerClient::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &MessengerClient::onDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &MessengerClient::onReadyRead);
    connect(socket, &QTcpSocket::errorOccurred, this, &MessengerClient::onError);
}

MessengerClient::~MessengerClient()
{
    if (connected) disconnectFromHost();
}

void MessengerClient::connectToServer(const QString& host, int port, const QString& user)
{
    username = user;
    socket->connectToHost(host, port);
}

void MessengerClient::disconnectFromHost()
{
    if (socket->state() == QAbstractSocket::ConnectedState) {
        socket->disconnectFromHost();
    }
    connected = false;
}

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
    msg.id = generateMessageId();

    std::string data = msg.serialize() + "\n";
    socket->write(data.c_str());
    socket->flush();

    qDebug() << "[DEBUG] Sent message to:" << to << " with id:" << msg.id.c_str();
}

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
    msg.reply_to = replyToId.toStdString();

    std::string data = msg.serialize() + "\n";
    socket->write(data.c_str());
    socket->flush();

    qDebug() << "[DEBUG] Sent reply to:" << to << " replying to:" << replyToId;
}

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
    msg.body = original.body;
    msg.timestamp = time(nullptr);
    msg.id = generateMessageId();
    msg.is_forwarded = true;
    msg.original_from = original.from;
    msg.original_body = original.body;

    std::string data = msg.serialize() + "\n";
    socket->write(data.c_str());
    socket->flush();

    qDebug() << "[DEBUG] Forwarded message to:" << to;
}

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

void MessengerClient::onConnected()
{
    connected = true;
    emit connectedToServer();
}

void MessengerClient::onDisconnected()
{
    connected = false;
    emit disconnectedFromServer();
}

void MessengerClient::onReadyRead()
{
    pendingData += QString::fromUtf8(socket->readAll());

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

void MessengerClient::parseIncomingData(const QString& line)
{
    qDebug() << "[DEBUG] Received:" << line;

    if (line.startsWith('{')) {
        try {
            Message msg = Message::deserialize(line.toStdString());

            if (msg.type == "auth_ok") {
                emit deliveryStatus(QString::fromStdString(msg.body));
                qDebug() << "[DEBUG] Authentication successful";
                requestUserList();
            }
            else if (msg.type == "user_list") {
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
                emit messageReceived(msg);
                qDebug() << "[DEBUG] Message from:" << msg.from.c_str() << " id:" << msg.id.c_str();
            }
            else if (msg.type == "history") {
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
                emit historyReceived({ QString::fromStdString(msg.body) });
            }
            else if (msg.type == "delivered") {
                emit deliveryStatus("Message delivered to " + QString::fromStdString(msg.to));
            }
            else if (msg.type == "error") {
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

void MessengerClient::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
        emit connectionError(socket->errorString());
}