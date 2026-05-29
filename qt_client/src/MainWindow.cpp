#include "MainWindow.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QApplication>
#include <QDateTime>
#include <QScrollBar>
#include <QClipboard>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QLabel>
#include "LoginDialog.h"
#include <QUuid>

MainWindow::MainWindow(const QString& serverIP,
    const QString& username,
    const QString& password,
    QWidget* parent)
    : QMainWindow(parent)
    , client(new MessengerClient(this))
{
    setWindowTitle("Messenger");
    resize(900, 650);

    // Стилизация главного окна и всех виджетов
    setStyleSheet(R"(
    QMainWindow {
        background-color: #f8f9fa;
    }
    QListWidget#userList {
        background-color: #ffffff;
        border-right: 1px solid #e0d5f0;
    }
    QListWidget#userList::item {
        padding: 12px;
        border-radius: 8px;
        margin: 2px 5px;
    }
    QListWidget#userList::item:hover {
        background-color: #f0e6ff;
    }
    QListWidget#userList::item:selected {
        background-color: #e8d5ff;
        color: #6b3fa0;
    }
    QListWidget#chatDisplay {
        background-color: #f8f9fa;
        border: none;
        outline: none;
    }
    QListWidget#chatDisplay::item {
        border: none;
        padding: 0px;
    }
    QLineEdit {
        background-color: #ffffff;
        color: #4a4a4a;
        border: 1px solid #e0d5f0;
        border-radius: 10px;
        padding: 10px;
        font-size: 14px;
    }
    QLineEdit:focus {
        border: 1px solid #c4a8e8;
    }
    QPushButton {
        background-color: #d4b8f0;
        color: #5a3a7a;
        border: none;
        border-radius: 10px;
        padding: 8px 20px;
        font-size: 14px;
    }
    QPushButton:hover {
        background-color: #c4a8e8;
        color: #4a2a6a;
    }
    QLabel#statusLabel {
        background-color: #f0eaff;
        color: #6b4a8a;
        padding: 8px;
        border-radius: 8px;
    }
    QMenu {
        background-color: #ffffff;
        color: #4a4a4a;
        border: 1px solid #e0d5f0;
        border-radius: 8px;
        padding: 5px;
    }
    QMenu::item {
        padding: 8px 20px;
        border-radius: 6px;
    }
    QMenu::item:selected {
        background-color: #f0e6ff;
        color: #6b3fa0;
    }
)");

    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Список пользователей (левая панель)
    userList = new QListWidget(this);
    userList->setObjectName("userList");
    userList->setFixedWidth(240);

    // Область отображения сообщений
    chatDisplay = new QListWidget(this);
    chatDisplay->setObjectName("chatDisplay");
    chatDisplay->setContextMenuPolicy(Qt::CustomContextMenu);  // Включаем контекстное меню
    chatDisplay->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    // Поле ввода сообщения
    messageInput = new QLineEdit(this);
    messageInput->setPlaceholderText("Type a message...");

    sendButton = new QPushButton("Send", this);

    // Статусная строка
    statusLabel = new QLabel("Connecting...", this);
    statusLabel->setObjectName("statusLabel");
    statusLabel->setFixedHeight(30);
    statusLabel->setAlignment(Qt::AlignCenter);

    // Компоновка области чата
    QVBoxLayout* chatLayout = new QVBoxLayout();
    chatLayout->addWidget(chatDisplay);
    chatLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* inputLayout = new QHBoxLayout();
    inputLayout->addWidget(messageInput);
    inputLayout->addWidget(sendButton);
    chatLayout->addLayout(inputLayout);

    QWidget* chatWidget = new QWidget();
    chatWidget->setLayout(chatLayout);

    // Разделитель между списком пользователей и чатом
    mainSplitter = new QSplitter(Qt::Horizontal);
    mainSplitter->addWidget(userList);
    mainSplitter->addWidget(chatWidget);
    mainSplitter->setStretchFactor(0, 0);
    mainSplitter->setStretchFactor(1, 1);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(mainSplitter);
    mainLayout->addWidget(statusLabel);

    // Подключение сигналов пользовательского интерфейса
    connect(userList, &QListWidget::clicked, this, &MainWindow::onUserSelected);
    connect(sendButton, &QPushButton::clicked, this, &MainWindow::onSendMessage);
    connect(messageInput, &QLineEdit::returnPressed, this, &MainWindow::onSendMessage);
    connect(chatDisplay, &QListWidget::customContextMenuRequested, this, &MainWindow::showContextMenu);

    // Подключение сигналов от сетевого клиента
    connect(client, &MessengerClient::messageReceived, this, &MainWindow::onMessageReceived);
    connect(client, &MessengerClient::userListReceived, this, &MainWindow::onUserListReceived);
    connect(client, &MessengerClient::historyReceived, this, &MainWindow::onHistoryReceived);
    connect(client, &MessengerClient::connectionError, this, &MainWindow::onConnectionStatus);
    connect(client, &MessengerClient::connectedToServer, this, &MainWindow::onConnected);
    connect(client, &MessengerClient::disconnectedFromServer, this, &MainWindow::onDisconnected);

    // После подключения к серверу отправляем логин и пароль
    connect(client, &MessengerClient::connectedToServer, this, [this, username, password]() {
        client->login(username, password);
        });

    client->connectToServer(serverIP, 7777, username);
}

MainWindow::~MainWindow() {}

// Успешное подключение к серверу
void MainWindow::onConnected()
{
    statusLabel->setText("✓ Connected as " + client->getUsername());
    statusLabel->setStyleSheet("background-color: #f8f9fa; color: #03965e;");
}

// Отключение от сервера
void MainWindow::onDisconnected()
{
    statusLabel->setText("✗ Disconnected");
    statusLabel->setStyleSheet("background-color: #f8f9fa; color: #ed4245;");
}

// Ошибка соединения или другое статусное сообщение
void MainWindow::onConnectionStatus(const QString& status)
{
    statusLabel->setText("⚠ " + status);
    statusLabel->setStyleSheet("background-color: #f8f9fa; color: #ab6d03;");
}

// Выбор пользователя из списка
void MainWindow::onUserSelected(const QModelIndex& index)
{
    currentChatUser = index.data().toString();
    setWindowTitle("Messenger - Chat with " + currentChatUser);

    // Снимаем выделение жирным шрифтом с выбранного пользователя
    for (int i = 0; i < userList->count(); ++i) {
        QListWidgetItem* item = userList->item(i);
        if (item->text() == currentChatUser) {
            QFont font = item->font();
            font.setBold(false);
            item->setFont(font);
            break;
        }
    }

    // Очищаем чат и кэш сообщений
    chatDisplay->clear();
    messageCache.clear();

    // Запрашиваем историю переписки
    client->requestHistory(currentChatUser, 50);
}

// Отправка сообщения
void MainWindow::onSendMessage()
{
    QString text = messageInput->text().trimmed();
    if (text.isEmpty()) return;
    if (currentChatUser.isEmpty()) {
        statusLabel->setText("⚠ Select a user first");
        return;
    }

    // Режим ответа на сообщение
    if (!pendingReplyId.isEmpty()) {
        client->sendReply(currentChatUser, text, pendingReplyId);
        Message localMsg;
        localMsg.id = QUuid::createUuid().toString().toStdString();
        localMsg.from = client->getUsername().toStdString();
        localMsg.to = currentChatUser.toStdString();
        localMsg.body = text.toStdString();
        localMsg.timestamp = QDateTime::currentSecsSinceEpoch();
        localMsg.reply_to = pendingReplyId.toStdString();

        appendMessage(localMsg, true);
        pendingReplyId.clear();
        messageInput->setPlaceholderText("Type a message...");
        statusLabel->setText("✓ Connected as " + client->getUsername());
    }
    else {
        // Обычная отправка
        client->sendMessage(currentChatUser, text);
        Message localMsg;
        localMsg.id = QUuid::createUuid().toString().toStdString();
        localMsg.from = client->getUsername().toStdString();
        localMsg.to = currentChatUser.toStdString();
        localMsg.body = text.toStdString();
        localMsg.timestamp = QDateTime::currentSecsSinceEpoch();

        appendMessage(localMsg, true);
    }
    messageInput->clear();
}

// Создание виджета для одного сообщения (с поддержкой reply и forward)
QWidget* MainWindow::createMessageWidget(const Message& msg, bool isMe, const QString& timeStr)
{
    QWidget* widget = new QWidget();
    widget->setAttribute(Qt::WA_StyledBackground, true);

    QVBoxLayout* layout = new QVBoxLayout(widget);
    layout->setContentsMargins(10, 5, 10, 5);
    layout->setSpacing(4);

    // Блок reply, если это ответ на другое сообщение
    if (!msg.reply_to.empty() && messageCache.contains(QString::fromStdString(msg.reply_to))) {
        Message& replied = messageCache[QString::fromStdString(msg.reply_to)];
        QLabel* replyLabel = new QLabel();
        replyLabel->setText(QString("↩ Reply to <b>%1</b>: %2")
            .arg(QString::fromStdString(replied.from))
            .arg(QString::fromStdString(replied.body).left(50)));
        replyLabel->setStyleSheet("background-color:#f0f0f0; color:#4a4a4a; border-radius:6px; padding:5px; font-size:11px; border-left: 3px solid #d4b8f0;");
        replyLabel->setWordWrap(true);
        layout->addWidget(replyLabel);
    }

    // Блок forward, если сообщение переслано
    if (msg.is_forwarded) {
        QLabel* forwardLabel = new QLabel();
        forwardLabel->setText(QString(" Forwarded from <b>%1</b>")
            .arg(QString::fromStdString(msg.original_from)));
        forwardLabel->setStyleSheet("background-color:#e8e0ff; color:#4a4a4a; border-radius:6px; padding:5px; font-size:11px; border-left: 3px solid #6b3fa0;");
        layout->addWidget(forwardLabel);
    }

    // Основной блок сообщения (пузырёк)
    QWidget* bubbleWidget = new QWidget();
    QHBoxLayout* bubbleLayout = new QHBoxLayout(bubbleWidget);
    bubbleLayout->setContentsMargins(0, 0, 0, 0);

    QLabel* messageLabel = new QLabel();
    QString senderName = isMe ? "You" : QString::fromStdString(msg.from);

    // Формируем HTML для сообщения
    QString messageHtml = QString(
        "<div style='max-width: 400px;'>"
        "<small><b>%1</b> · %2</small><br>"
        "<span style='font-size: 14px;'>%3</span>"
        "</div>"
    ).arg(senderName.toHtmlEscaped(), timeStr, QString::fromStdString(msg.body).toHtmlEscaped());

    messageLabel->setText(messageHtml);
    messageLabel->setWordWrap(true);
    messageLabel->setTextFormat(Qt::RichText);

    // Стилизация пузырька в зависимости от того, своё сообщение или чужое
    if (isMe) {
        messageLabel->setStyleSheet(
            "background-color:#d4b8f0; "
            "color:#2d1f3a; "
            "border-radius:12px; "
            "padding:8px; "
            "margin:2px;"
        );
        bubbleLayout->addStretch();      // Свои сообщения прижимаем к правому краю
        bubbleLayout->addWidget(messageLabel);
    }
    else {
        messageLabel->setStyleSheet(
            "background-color:#f0f0f0; "
            "color:#2d2d3a; "
            "border-radius:12px; "
            "padding:8px; "
            "margin:2px;"
        );
        bubbleLayout->addWidget(messageLabel);
        bubbleLayout->addStretch();      // Чужие сообщения прижимаем к левому краю
    }

    layout->addWidget(bubbleWidget);
    widget->setStyleSheet("background-color: transparent;");

    return widget;
}

// Добавление сообщения в чат
void MainWindow::appendMessage(const Message& msg, bool isMe)
{
    QString timeStr = QDateTime::fromSecsSinceEpoch(msg.timestamp)
        .toString("hh:mm:ss");

    QWidget* widget = createMessageWidget(msg, isMe, timeStr);
    QListWidgetItem* item = new QListWidgetItem();

    // Сохраняем ID сообщения в UserRole для контекстного меню
    item->setData(Qt::UserRole, QString::fromStdString(msg.id));

    item->setSizeHint(widget->sizeHint());
    chatDisplay->addItem(item);
    chatDisplay->setItemWidget(item, widget);

    // Автоматическая прокрутка вниз
    chatDisplay->scrollToBottom();
}

// Получение нового сообщения от сервера
void MainWindow::onMessageReceived(const Message& msg)
{
    messageCache[QString::fromStdString(msg.id)] = msg;

    bool isMe = (msg.from == client->getUsername().toStdString());

    // Если чат открыт с этим пользователем — показываем сразу
    if (QString::fromStdString(msg.from) == currentChatUser ||
        (isMe && QString::fromStdString(msg.to) == currentChatUser)) {
        appendMessage(msg, isMe);
    }
    else {
        // Иначе подсвечиваем имя пользователя в списке жирным шрифтом
        QString fromUser = QString::fromStdString(msg.from);
        for (int i = 0; i < userList->count(); ++i) {
            QListWidgetItem* item = userList->item(i);
            if (item->text() == fromUser) {
                QFont font = item->font();
                font.setBold(true);
                item->setFont(font);
                break;
            }
        }
    }
}

// Обновление списка пользователей
void MainWindow::onUserListReceived(const QStringList& users)
{
    userList->clear();
    for (QString user : users) {
        bool online = false;
        if (user.endsWith("*")) {
            online = true;
            user.chop(1);  // Убираем звёздочку
        }
        if (user == client->getUsername()) continue;

        QListWidgetItem* item = new QListWidgetItem(user);
        if (online) {
            item->setForeground(QColor("#03965e"));  // Зелёный для онлайн
        }
        else {
            item->setForeground(QColor("#888888"));  // Серый для офлайн
        }
        userList->addItem(item);
    }
}

// Получение истории переписки
void MainWindow::onHistoryReceived(const QStringList& historyLines)
{
    for (const QString& line : historyLines) {
        QString time = line.mid(1, 8);
        int colonPos = line.indexOf(": ", 11);
        if (colonPos != -1) {
            QString sender = line.mid(11, colonPos - 11);
            QString message = line.mid(colonPos + 2);

            QString msgId, replyToId;
            bool isForwarded = false;
            QString originalFrom;

            // Извлекаем метаданные из строки (формат с разделителем |||)
            if (message.contains("|||")) {
                QStringList parts = message.split("|||");
                message = parts[0];
                if (parts.size() > 1) msgId = parts[1];
                if (parts.size() > 2) replyToId = parts[2];
                if (parts.size() > 3) isForwarded = (parts[3] == "1");
                if (parts.size() > 4) originalFrom = parts[4];
            }

            // Парсим время
            QTime parsedTime = QTime::fromString(time, "hh:mm:ss");
            QDateTime dateTime = QDateTime::currentDateTime();
            dateTime.setTime(parsedTime);

            Message histMsg;
            histMsg.id = msgId.toStdString();
            histMsg.from = sender.toStdString();
            histMsg.to = currentChatUser.toStdString();
            histMsg.body = message.toStdString();
            histMsg.timestamp = dateTime.toSecsSinceEpoch();
            histMsg.reply_to = replyToId.toStdString();
            histMsg.is_forwarded = isForwarded;
            histMsg.original_from = originalFrom.toStdString();

            bool isMe = (sender == client->getUsername());
            messageCache[msgId] = histMsg;
            appendMessage(histMsg, isMe);
        }
    }
    chatDisplay->scrollToBottom();
}

// Показ контекстного меню (правой кнопкой мыши по сообщению)
void MainWindow::showContextMenu(const QPoint& pos)
{
    QListWidgetItem* item = chatDisplay->itemAt(pos);
    if (!item) return;

    selectedMessageId = item->data(Qt::UserRole).toString();

    if (!messageCache.contains(selectedMessageId)) return;

    selectedMessage = messageCache[selectedMessageId];

    QMenu menu(this);
    QAction* replyAction = menu.addAction("↩Reply");
    QAction* forwardAction = menu.addAction("Forward");
    menu.addSeparator();
    QAction* copyAction = menu.addAction("Copy");

    QAction* selected = menu.exec(chatDisplay->viewport()->mapToGlobal(pos));

    if (selected == replyAction) {
        onReplyToMessage();
    }
    else if (selected == forwardAction) {
        onForwardMessage();
    }
    else if (selected == copyAction) {
        onCopyMessage();
    }
}

// Активация режима ответа на сообщение
void MainWindow::onReplyToMessage()
{
    if (!selectedMessageId.isEmpty()) {
        pendingReplyId = selectedMessageId;
        messageInput->setPlaceholderText("↩ Replying to message... (Press ESC to cancel)");
        statusLabel->setText("↩ Replying... Press ESC to cancel");
        messageInput->setFocus();
    }
}

// Пересылка сообщения другому пользователю
void MainWindow::onForwardMessage()
{
    if (selectedMessageId.isEmpty()) return;

    // Собираем список пользователей для выбора
    QStringList users;
    for (int i = 0; i < userList->count(); ++i) {
        users << userList->item(i)->text();
    }

    bool ok;
    QString targetUser = QInputDialog::getItem(this, "Forward Message",
        "Select user to forward to:", users, 0, false, &ok);

    if (ok && !targetUser.isEmpty()) {
        client->forwardMessage(targetUser, selectedMessage);
        statusLabel->setText("✓ Message forwarded to " + targetUser);
    }
}

// Копирование текста сообщения в буфер обмена
void MainWindow::onCopyMessage()
{
    if (!selectedMessageId.isEmpty() && messageCache.contains(selectedMessageId)) {
        QString text = QString::fromStdString(selectedMessage.body);
        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setText(text);
        statusLabel->setText("Message copied to clipboard");
    }
}

// Обработка нажатий клавиш
void MainWindow::keyPressEvent(QKeyEvent* event)
{
    // ESC отменяет режим ответа
    if (event->key() == Qt::Key_Escape && !pendingReplyId.isEmpty()) {
        pendingReplyId.clear();
        messageInput->setPlaceholderText("Type a message...");
        statusLabel->setText("✓ Connected as " + client->getUsername());
    }
    QMainWindow::keyPressEvent(event);
}