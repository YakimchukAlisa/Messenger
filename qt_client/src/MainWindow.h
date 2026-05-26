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

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    MessengerClient* client;

    QWidget* centralWidget;
    QSplitter* mainSplitter;

    QListWidget* userList;      // —лева Ч список пользователей
    QTextEdit* chatDisplay;     // —права сверху Ч окно чата
    QLineEdit* messageInput;    // —права снизу Ч поле ввода
    QPushButton* sendButton;
    QLabel* statusLabel;

    QString currentChatUser;    // — кем сейчас открыт чат
    QStringList currentHistory; // »стори€ текущего чата

    void updateChatDisplay();

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onUserSelected(const QModelIndex& index);
    void onSendMessage();
    void onMessageReceived(const QString& from, const QString& body, time_t timestamp);
    void onUserListReceived(const QStringList& users);
    void onHistoryReceived(const QStringList& historyLines);
    void onConnectionStatus(const QString& status);
    void onConnected();
    void onDisconnected();
};

#endif