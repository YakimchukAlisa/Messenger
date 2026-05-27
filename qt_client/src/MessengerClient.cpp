#include "MessengerClient.h"
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
    std::string loginData = username.toStdString() + "|" + password.toStdString() + "\n";
    socket->write(loginData.c_str());
    socket->flush();
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

    std::string data = msg.serialize() + "\n";
    socket->write(data.c_str());
    socket->flush();
}

void MessengerClient::requestHistory(const QString& withUser, int limit)
{
    if (!connected) return;

    Message msg;
    msg.type = "history";
    msg.from = username.toStdString();
    msg.to = withUser.toStdString();
    msg.body = std::to_string(limit);
    msg.timestamp = time(nullptr);

    std::string data = msg.serialize() + "\n";
    socket->write(data.c_str());
    socket->flush();
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

        parseIncomingData(line);
    }
}

void MessengerClient::parseIncomingData(const QString& line)
{
    if (line.startsWith("/users")) {
        QStringList rawUsers = line.mid(7).split(' ', Qt::SkipEmptyParts);
        rawUsers.removeAll(username);
        rawUsers.removeAll(username + "*");
        emit userListReceived(rawUsers);
    }
    else if (line.startsWith("/history_line ")) {
        QString historyLine = line.mid(14);
        emit historyReceived({ historyLine });
    }
    else if (line.startsWith("/error")) {
        emit deliveryStatus("Error: " + line.mid(7));
    }
    else if (line.startsWith("/delivered")) {
        emit deliveryStatus("Message delivered");
    }
    else if (line.startsWith("/welcome")) {
        // Успешный логин
        emit deliveryStatus("Login successful");
    }
    else {
        try {
            Message msg = Message::deserialize(line.toStdString());
            emit messageReceived(
                QString::fromStdString(msg.from),
                QString::fromStdString(msg.body),
                msg.timestamp
            );
        }
        catch (...) {
            qDebug() << "Unknown message:" << line;
        }
    }
}

void MessengerClient::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
        emit connectionError(socket->errorString());
}