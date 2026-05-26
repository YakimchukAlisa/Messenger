#include "MainWindow.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QApplication>
#include <QDateTime>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , client(new MessengerClient(this))
{
    setWindowTitle("Messenger");
    resize(800, 600);

    // Создаём виджеты
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    userList = new QListWidget(this);
    userList->setFixedWidth(200);

    chatDisplay = new QTextEdit(this);
    chatDisplay->setReadOnly(true);

    messageInput = new QLineEdit(this);
    messageInput->setPlaceholderText("Type a message...");

    sendButton = new QPushButton("Send", this);

    statusLabel = new QLabel("Connecting...", this);
    statusLabel->setFixedHeight(20);

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
    statusLabel->setText("Connected as " + client->getUsername());
    statusLabel->setStyleSheet("color: green;");
}

void MainWindow::onDisconnected()
{
    statusLabel->setText("Disconnected");
    statusLabel->setStyleSheet("color: red;");
}

void MainWindow::onConnectionStatus(const QString& status)
{
    statusLabel->setText(status);
    statusLabel->setStyleSheet("color: orange;");
}

void MainWindow::onUserSelected(const QModelIndex& index)
{
    currentChatUser = index.data().toString();
    setWindowTitle("Messenger - Chat with " + currentChatUser);

    // СБРАСЫВАЕМ ПОДСВЕТКУ для выбранного пользователя
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
        statusLabel->setText("Select a user first!");
        return;
    }

    client->sendMessage(currentChatUser, text);
    messageInput->clear();

    // Отображаем своё сообщение сразу
    QDateTime now = QDateTime::currentDateTime();
    chatDisplay->append(QString("<b><font color='blue'>[%1] Me:</font></b> %2")
        .arg(now.toString("hh:mm:ss"))
        .arg(text.toHtmlEscaped()));
}

void MainWindow::onUserListReceived(const QStringList& users)
{
    userList->clear();

    for (const QString& user : users) {
        QListWidgetItem* item = new QListWidgetItem(user);

        if (user.endsWith("*")) {
            // Онлайн — зелёный
            item->setForeground(Qt::darkGreen);
            item->setText(user.left(user.length() - 1));
        }
        else {
            // Офлайн — серый
            item->setForeground(Qt::gray);
        }

        userList->addItem(item);
    }
}

void MainWindow::onMessageReceived(const QString& from, const QString& body, long timestamp)
{
    char timeStr[64];
    time_t ts = (time_t)timestamp;
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", localtime(&ts));

    // Если чат открыт с этим пользователем — показываем сразу
    if (from == currentChatUser) {
        chatDisplay->append(QString("<b><font color='green'>[%1] %2:</font></b> %3")
            .arg(timeStr)
            .arg(from.toHtmlEscaped())
            .arg(body.toHtmlEscaped()));
    }
    else {

        // Добавляем жирный шрифт к имени пользователя в списке
        for (int i = 0; i < userList->count(); ++i) {
            QListWidgetItem* item = userList->item(i);
            if (item->text() == from) {
                QFont font = item->font();
                font.setBold(true);
                item->setFont(font);
                break;
            }
        }
    }
}

void MainWindow::onHistoryReceived(const QStringList& historyLines)
{
    for (const QString& line : historyLines) {
        chatDisplay->append(line);
    }
}

void MainWindow::updateChatDisplay()
{
    // Не используется, история приходит через сигнал
}