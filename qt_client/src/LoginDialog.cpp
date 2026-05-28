#include "LoginDialog.h"
#include <QTimer>

LoginDialog::LoginDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Messenger - Login");
    resize(900, 650);

    setStyleSheet(R"(
        QDialog {
            background-color: #f8f9fa;
        }
        QLabel {
            color: #6b4a8a;
            font-size: 13px;
            font-weight: 500;
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
            outline: none;
        }
        QPushButton {
            background-color: #d4b8f0;
            color: #5a3a7a;
            border: none;
            border-radius: 10px;
            padding: 10px 20px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #c4a8e8;
            color: #4a2a6a;
        }
        QPushButton#cancelButton {
            background-color: #e8e0f0;
            color: #6b4a8a;
        }
        QPushButton#cancelButton:hover {
            background-color: #ddd0f0;
            color: #5a3a7a;
        }
        QLabel#titleLabel {
            font-size: 24px;
            font-weight: bold;
            color: #6b3fa0;
            margin-bottom: 20px;
        }
        QLabel#statusLabel {
            color: #d32f2f;
            font-size: 12px;
            background-color: #ffebee;
            padding: 8px;
            border-radius: 8px;
            border: 1px solid #ffcdd2;
        }
    )");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(40, 40, 40, 40);

    // Заголовок
    QLabel* titleLabel = new QLabel("Messenger");
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // Подзаголовок
    QLabel* subtitleLabel = new QLabel("Welcome back! Please login or register");
    subtitleLabel->setStyleSheet("color: #8e6eb0; font-size: 12px; margin-bottom: 20px;");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(subtitleLabel);

    // Сервер
    QLabel* serverLabel = new QLabel("Server IP:");
    serverIpEdit = new QLineEdit();
    serverIpEdit->setPlaceholderText("127.0.0.1");
    serverIpEdit->setText("127.0.0.1");
    mainLayout->addWidget(serverLabel);
    mainLayout->addWidget(serverIpEdit);

    // Логин
    QLabel* usernameLabel = new QLabel("Username:");
    usernameEdit = new QLineEdit();
    usernameEdit->setPlaceholderText("Enter your username");
    mainLayout->addWidget(usernameLabel);
    mainLayout->addWidget(usernameEdit);

    // Пароль
    QLabel* passwordLabel = new QLabel("Password:");
    passwordEdit = new QLineEdit();
    passwordEdit->setPlaceholderText("Enter your password");
    passwordEdit->setEchoMode(QLineEdit::Password);
    mainLayout->addWidget(passwordLabel);
    mainLayout->addWidget(passwordEdit);

    // Статус ошибки
    statusLabel = new QLabel();
    statusLabel->setObjectName("statusLabel");
    statusLabel->setVisible(false);
    statusLabel->setWordWrap(true);
    mainLayout->addWidget(statusLabel);

    // Кнопки
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    cancelButton = new QPushButton("Cancel");
    cancelButton->setObjectName("cancelButton");
    loginButton = new QPushButton("Login / Register");

    // ОТКЛЮЧАЕМ АВТОМАТИЧЕСКОЕ ПОВЕДЕНИЕ ENTER
    loginButton->setDefault(false);
    loginButton->setAutoDefault(false);
    cancelButton->setDefault(false);
    cancelButton->setAutoDefault(false);

    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(loginButton);
    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);


    // Сигналы
    connect(loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    connect(cancelButton, &QPushButton::clicked, this, &LoginDialog::onCancelClicked);

    // Enter через QTimer, чтобы не закрывало окно до проверки
    connect(usernameEdit, &QLineEdit::returnPressed, this, [this]() {
        QTimer::singleShot(0, this, &LoginDialog::onLoginClicked);
        });
    connect(passwordEdit, &QLineEdit::returnPressed, this, [this]() {
        QTimer::singleShot(0, this, &LoginDialog::onLoginClicked);
        });

    usernameEdit->setFocus();
}

void LoginDialog::onLoginClicked()
{
    QString username = usernameEdit->text().trimmed();
    QString password = passwordEdit->text();

    if (username.isEmpty()) {
        showError("Username cannot be empty");
        usernameEdit->setFocus();
        return;
    }

    if (password.isEmpty()) {
        showError("Password cannot be empty");
        passwordEdit->setFocus();
        return;
    }


    // Все проверки пройдены
    accept();
}

void LoginDialog::onCancelClicked()
{
    reject();
}

QString LoginDialog::getServerIp() const
{
    return serverIpEdit->text().trimmed();
}

QString LoginDialog::getUsername() const
{
    return usernameEdit->text().trimmed();
}

QString LoginDialog::getPassword() const
{
    return passwordEdit->text();
}

void LoginDialog::showError(const QString& message)
{
    statusLabel->setText(message);
    statusLabel->show();
    statusLabel->raise();
    statusLabel->update();

    // Автоматически скрыть через 3 секунды
    QTimer::singleShot(3000, this, [this]() {
        if (statusLabel) {
            statusLabel->hide();
        }
        });
}

void LoginDialog::clearError()
{
    statusLabel->hide();
}