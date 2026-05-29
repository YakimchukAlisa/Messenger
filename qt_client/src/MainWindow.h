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

// Главное окно мессенджера
// Отвечает за отображение списка пользователей, чата,
// отправку сообщений, контекстное меню (reply/forward/copy)
class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    // Сетевой клиент
    MessengerClient* client;

    // Базовые виджеты
    QWidget* centralWidget;
    QSplitter* mainSplitter;          // Разделитель между списком пользователей и чатом

    // Элементы интерфейса
    QListWidget* userList;            // Список пользователей (слева)
    QListWidget* chatDisplay;         // Область отображения сообщений (справа)
    QLineEdit* messageInput;          // Поле ввода сообщения
    QPushButton* sendButton;          // Кнопка отправки
    QLabel* statusLabel;              // Статусная строка

    // Текущий выбранный пользователь для чата
    QString currentChatUser;

    // Кэш сообщений (ID сообщения -> само сообщение)
    QMap<QString, Message> messageCache;
    QString pendingReplyId;           // ID сообщения, на которое готовится ответ

    // Выбранное сообщение для контекстного меню
    QString selectedMessageId;
    Message selectedMessage;

    // Вспомогательные методы
    void appendMessage(const Message& msg, bool isMe);               // Добавление сообщения в чат
    QWidget* createMessageWidget(const Message& msg, bool isMe, const QString& timeStr);  // Создание виджета сообщения

public:
    // Конструктор принимает IP сервера, имя пользователя и пароль из диалога логина
    MainWindow(const QString& serverIP,
        const QString& username,
        const QString& password,
        QWidget* parent = nullptr);
    ~MainWindow();

protected:
    // Обработка нажатий клавиш (ESC для отмены ответа)
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    // Обработчики событий интерфейса
    void onUserSelected(const QModelIndex& index);   // Выбор пользователя из списка
    void onSendMessage();                            // Отправка сообщения

    // Обработчики событий от сетевого клиента
    void onMessageReceived(const Message& msg);      // Получено новое сообщение
    void onUserListReceived(const QStringList& users);  // Обновление списка пользователей
    void onHistoryReceived(const QStringList& historyLines);  // Получена история переписки
    void onConnectionStatus(const QString& status);  // Изменение статуса соединения
    void onConnected();                              // Подключение к серверу установлено
    void onDisconnected();                           // Отключение от сервера

    // Контекстное меню (правой кнопкой мыши по сообщению)
    void showContextMenu(const QPoint& pos);         // Показ меню
    void onReplyToMessage();                         // Ответ на выбранное сообщение
    void onForwardMessage();                         // Пересылка выбранного сообщения
    void onCopyMessage();                            // Копирование текста выбранного сообщения
};

#endif