#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDateTime>
#include <QStringList>
#include <QMap>
#include <QKeyEvent>
#include <QMenu>
#include <QPoint>
#include "MessengerClient.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    MessengerClient* client;

    QWidget* centralWidget;
    QSplitter* mainSplitter;

    QListWidget* userList;
    QListWidget* chatDisplay;  // ← ИЗМЕНЕНО: теперь QListWidget
    QLineEdit* messageInput;
    QPushButton* sendButton;
    QLabel* statusLabel;

    QString currentChatUser;

    QMap<QString, Message> messageCache;
    QString pendingReplyId;

    QString selectedMessageId;
    Message selectedMessage;

    void appendMessage(const Message& msg, bool isMe);
    QWidget* createMessageWidget(const Message& msg, bool isMe, const QString& timeStr);

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void onUserSelected(const QModelIndex& index);
    void onSendMessage();
    void onMessageReceived(const Message& msg);
    void onUserListReceived(const QStringList& users);
    void onHistoryReceived(const QStringList& historyLines);
    void onConnectionStatus(const QString& status);
    void onConnected();
    void onDisconnected();

    void showContextMenu(const QPoint& pos);
    void onReplyToMessage();
    void onForwardMessage();
    void onCopyMessage();
};

#endif