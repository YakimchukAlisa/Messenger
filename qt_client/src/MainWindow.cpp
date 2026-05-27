#include "MainWindow.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QApplication>
#include <QDateTime>
#include <QScrollBar>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , client(new MessengerClient(this))
{
    setWindowTitle("Messenger");
    resize(900, 650);

    // Устанавливаем стиль для всего окна
    setStyleSheet(R"(
    QMainWindow {
        background-color: #f8f9fa;
    }
    QListWidget {
        background-color: #ffffff;
        color: #4a4a4a;
        border: none;
        font-size: 14px;
        outline: none;
        border-radius: 10px;
        margin: 5px;
    }
    QListWidget::item {
        padding: 10px;
        border-radius: 8px;
        margin: 2px 5px;
    }
    QListWidget::item:hover {
        background-color: #f0e6ff;
    }
    QListWidget::item:selected {
        background-color: #e8d5ff;
        color: #6b3fa0;
    }
    QTextEdit {
        background-color: #ffffff;
        color: #4a4a4a;
        border: none;
        font-size: 14px;
        border-radius: 10px;
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
    QLabel {
        background-color: #f0eaff;
        color: #6b4a8a;
        padding: 8px;
        border-radius: 8px;
    }
)");

    // Создаём виджеты
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    userList = new QListWidget(this);
    userList->setFixedWidth(240);

    chatDisplay = new QTextEdit(this);
    chatDisplay->setReadOnly(true);
    chatDisplay->setStyleSheet(R"(
        QTextEdit {
            background-color: #ffffff;
            color: #dcddde;
            border: none;
        }
    )");

    messageInput = new QLineEdit(this);
    messageInput->setPlaceholderText("Type a message...");

    sendButton = new QPushButton("Send", this);

    statusLabel = new QLabel("Connecting...", this);
    statusLabel->setFixedHeight(30);
    statusLabel->setAlignment(Qt::AlignCenter);

    // Layout
    QVBoxLayout* chatLayout = new QVBoxLayout();
    chatLayout->addWidget(chatDisplay);

    QHBoxLayout* inputLayout = new QHBoxLayout();
    inputLayout->addWidget(messageInput);
    inputLayout->addWidget(sendButton);
    chatLayout->addLayout(inputLayout);

    QWidget* chatWidget = new QWidget();
    chatWidget->setLayout(chatLayout);

    mainSplitter = new QSplitter(Qt::Horizontal);
    mainSplitter->addWidget(userList);
    mainSplitter->addWidget(chatWidget);
    mainSplitter->setStretchFactor(0, 0);
    mainSplitter->setStretchFactor(1, 1);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(mainSplitter);
    mainLayout->addWidget(statusLabel);

    // Подключаем сигналы
    connect(userList, &QListWidget::clicked, this, &MainWindow::onUserSelected);
    connect(sendButton, &QPushButton::clicked, this, &MainWindow::onSendMessage);
    connect(messageInput, &QLineEdit::returnPressed, this, &MainWindow::onSendMessage);

    connect(client, &MessengerClient::messageReceived, this, &MainWindow::onMessageReceived);
    connect(client, &MessengerClient::userListReceived, this, &MainWindow::onUserListReceived);
    connect(client, &MessengerClient::historyReceived, this, &MainWindow::onHistoryReceived);
    connect(client, &MessengerClient::connectionError, this, &MainWindow::onConnectionStatus);
    connect(client, &MessengerClient::connectedToServer, this, &MainWindow::onConnected);
    connect(client, &MessengerClient::disconnectedFromServer, this, &MainWindow::onDisconnected);

    // Запрашиваем сервер и имя
    bool ok;
    QString serverIP = QInputDialog::getText(this, "Connect", "Server IP:", QLineEdit::Normal, "127.0.0.1", &ok);
    if (!ok) {
        QApplication::quit();
        return;
    }

    QString username = QInputDialog::getText(this, "Connect", "Username:", QLineEdit::Normal, "", &ok);
    if (!ok || username.isEmpty()) {
        QApplication::quit();
        return;
    }

    client->connectToServer(serverIP, 7777, username);
}

MainWindow::~MainWindow() {}

void MainWindow::onConnected()
{
    statusLabel->setText("✓ Connected as " + client->getUsername());
    statusLabel->setStyleSheet("background-color: #f8f9fa; color: #03965e;");
}

void MainWindow::onDisconnected()
{
    statusLabel->setText("✗ Disconnected");
    statusLabel->setStyleSheet("background-color: #f8f9fa; color: #ed4245;");
}

void MainWindow::onConnectionStatus(const QString& status)
{
    statusLabel->setText("⚠ " + status);
    statusLabel->setStyleSheet("background-color: #f8f9fa; color: #ab6d03;");
}

void MainWindow::onUserSelected(const QModelIndex& index)
{
    currentChatUser = index.data().toString();
    setWindowTitle("Messenger - Chat with " + currentChatUser);

    // Сбрасываем подсветку для выбранного пользователя
    for (int i = 0; i < userList->count(); ++i) {
        QListWidgetItem* item = userList->item(i);
        if (item->text() == currentChatUser) {
            QFont font = item->font();
            font.setBold(false);
            item->setFont(font);
            break;
        }
    }

    chatDisplay->clear();

    // Запрашиваем историю с сервера
    client->requestHistory(currentChatUser, 50);
}

void MainWindow::onSendMessage()
{
    QString text = messageInput->text().trimmed();
    if (text.isEmpty()) return;
    if (currentChatUser.isEmpty()) {
        statusLabel->setText("⚠ Select a user first!");
        return;
    }

    client->sendMessage(currentChatUser, text);
    messageInput->clear();

    // Отображаем своё сообщение в цветном пузыре
    QDateTime now = QDateTime::currentDateTime();
    appendMessage("Me", text, now.toString("hh:mm:ss"), true);

    // Прокручиваем вниз
    QScrollBar* scrollBar = chatDisplay->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void MainWindow::appendMessage(const QString& sender, const QString& message,
    const QString& time, bool isMe)
{
    QString html;

    if (isMe) {
        // Моё сообщение — нежный фиолетовый
        html = QString(R"(
<table width="100%">
<tr>
    <td align="right">
        <table cellpadding="8">
        <tr>
            <td bgcolor="#d4b8f0"
                style="color:#4a3a6a;
                       border-radius:12px;">
                <small>You · %1</small><br>
                %2
            </td>
        </tr>
        </table>
    </td>
</tr>
</table>
)")
.arg(time, message.toHtmlEscaped());
    }
    else {
        // Сообщение собеседника — нежно-серый
        html = QString(R"(
<table width="100%">
<tr>
    <td align="left">
        <table cellpadding="8">
        <tr>
            <td bgcolor="#f0f0f0"
                style="color:#6a6a7a;
                       border-radius:12px;">
                <small>%1 · %2</small><br>
                %3
            </td>
        </tr>
        </table>
    </td>
</tr>
</table>
)")
.arg(sender.toHtmlEscaped(), time, message.toHtmlEscaped());
    }

    chatDisplay->append(html);
}

void MainWindow::onMessageReceived(const QString& from, const QString& body, long timestamp)
{
    char timeStr[64];
    time_t ts = (time_t)timestamp;
    struct tm* timeInfo = localtime(&ts);
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", timeInfo);

    // Если чат открыт с этим пользователем — показываем сразу
    if (from == currentChatUser) {
        appendMessage(from, body, QString(timeStr), false);

        // Прокручиваем вниз
        QScrollBar* scrollBar = chatDisplay->verticalScrollBar();
        scrollBar->setValue(scrollBar->maximum());
    }
    else {
        // Подсвечиваем в списке пользователей
        for (int i = 0; i < userList->count(); ++i) {
            QListWidgetItem* item = userList->item(i);
            if (item->text() == from) {
                QFont font = item->font();
                font.setBold(true);
                item->setFont(font);
                break;
            }
        }

        // Сбрасываем цвет статуса через 3 секунды
        QTimer::singleShot(3000, this, [this]() {
            if (client->isConnected()) {
                statusLabel->setText("✓ Connected as " + client->getUsername());
                statusLabel->setStyleSheet("background-color: #2f3136; color: #57f287;");
            }
            });
    }
}

void MainWindow::onUserListReceived(const QStringList& users)
{
    userList->clear();

    for (QString user : users) {

        bool online = false;

        if (user.endsWith("*")) {
            online = true;
            user.chop(1); 
        }

        // Не показываем себя
        if (user == client->getUsername())
            continue;

        QListWidgetItem* item = new QListWidgetItem(user);

        if (online)
            item->setForeground(QColor("#03965e"));
        else
            item->setForeground(QColor("#888888"));

        userList->addItem(item);
    }
}

void MainWindow::onHistoryReceived(const QStringList& historyLines)
{
    for (const QString& line : historyLines) {
        // Парсим строку истории вида: "[HH:MM:SS] имя: сообщение"
        QString time = line.mid(1, 8);  // "HH:MM:SS"
        int colonPos = line.indexOf(": ", 11);
        if (colonPos != -1) {
            QString sender = line.mid(11, colonPos - 11);
            QString message = line.mid(colonPos + 2);

            bool isMe = (sender == client->getUsername());
            appendMessage(sender, message, time, isMe);
        }
        else {
            // Если формат не совпадает, выводим как есть
            chatDisplay->append(line);
        }
    }

    // Прокручиваем вниз после загрузки истории
    QScrollBar* scrollBar = chatDisplay->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

