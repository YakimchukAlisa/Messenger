#ifndef MESSENGERCLIENT_H
#define MESSENGERCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QThread>
#include <QStringList>
#include "../../common/Protocol.h"
#include "../../common/Database.h"

class MessengerClient : public QObject
{
    Q_OBJECT

private:
    QTcpSocket* socket;
    QString username;
    bool connected;
    
    QString pendingData;
    
    void parseIncomingData(const QString& data);

public:
    explicit MessengerClient(QObject* parent = nullptr);
    ~MessengerClient();
    
    void connectToServer(const QString& host, int port, const QString& user);
    void disconnect();
    void sendMessage(const QString& to, const QString& body);
    void requestHistory(const QString& withUser, int limit = 50);
    
    bool isConnected() const { return connected; }
    QString getUsername() const { return username; }

signals:
    void connectedToServer();
    void disconnectedFromServer();
    void connectionError(const QString& error);
    
    void messageReceived(const QString& from, const QString& body, long timestamp);
    void userListReceived(const QStringList& users);
    void historyReceived(const QStringList& historyLines);
    void deliveryStatus(const QString& status);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError error);
};

#endif
