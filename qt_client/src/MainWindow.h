#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDateTime>
#include <QStringList>
#include "MessengerClient.h"
#include <QTimer>  

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    MessengerClient* client;

    QWidget* centralWidget;
    QSplitter* mainSplitter;

    QListWidget* userList;      // Слева — список пользователей
    QTextEdit* chatDisplay;     // Справа сверху — окно чата
    QLineEdit* messageInput;    // Справа снизу — поле ввода
    QPushButton* sendButton;
    QLabel* statusLabel;

    QString currentChatUser;    // С кем сейчас открыт чат
    QStringList currentHistory; // История текущего чата

    void updateChatDisplay();

    void appendMessage(const QString& sender, const QString& message,
        const QString& time, bool isMe);

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onUserSelected(const QModelIndex& index);
    void onSendMessage();
    void onMessageReceived(const QString& from, const QString& body, long timestamp);
    void onUserListReceived(const QStringList& users);
    void onHistoryReceived(const QStringList& historyLines);
    void onConnectionStatus(const QString& status);
    void onConnected();
    void onDisconnected();
};

#endif