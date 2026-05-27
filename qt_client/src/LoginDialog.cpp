#include "LoginDialog.h"

LoginDialog::LoginDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Messenger - Login");
    resize(900, 650);

    setStyleSheet(R"(
        QDialog {
            background-color: #36393f;
        }
        QLabel {
            color: #dcddde;
            font-size: 13px;
        }
        QLineEdit {
            background-color: #40444b;
            color: #dcddde;
            border: none;
            border-radius: 6px;
            padding: 10px;
            font-size: 14px;
        }
        QPushButton {
            background-color: #5865f2;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 16px;
            font-size: 14px;
        }
        QPushButton:hover {
            background-color: #4752c4;
        }
        QPushButton#cancelButton {
            background-color: #4f545c;
        }
        QPushButton#cancelButton:hover {
            background-color: #5d626b;
        }
    )");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);

    QLabel* titleLabel = new QLabel("Welcome to Messenger");
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #ffffff; margin-bottom: 10px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    QLabel* serverLabel = new QLabel("Server IP:");
    serverIpEdit = new QLineEdit();
    serverIpEdit->setPlaceholderText("127.0.0.1");
    serverIpEdit->setText("127.0.0.1");
    mainLayout->addWidget(serverLabel);
    mainLayout->addWidget(serverIpEdit);

    QLabel* usernameLabel = new QLabel("Username:");
    usernameEdit = new QLineEdit();
    usernameEdit->setPlaceholderText("Enter your username");
    mainLayout->addWidget(usernameLabel);
    mainLayout->addWidget(usernameEdit);

    QLabel* passwordLabel = new QLabel("Password:");
    passwordEdit = new QLineEdit();
    passwordEdit->setPlaceholderText("Enter your password");
    passwordEdit->setEchoMode(QLineEdit::Password);
    mainLayout->addWidget(passwordLabel);
    mainLayout->addWidget(passwordEdit);

    statusLabel = new QLabel();
    statusLabel->setStyleSheet("color: #ed4245; font-size: 12px;");
    statusLabel->setVisible(false);
    mainLayout->addWidget(statusLabel);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    cancelButton = new QPushButton("Cancel");
    cancelButton->setObjectName("cancelButton");
    loginButton = new QPushButton("Login");

    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(loginButton);
    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);

    connect(loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    connect(cancelButton, &QPushButton::clicked, this, &LoginDialog::onCancelClicked);
    connect(usernameEdit, &QLineEdit::returnPressed, this, &LoginDialog::onLoginClicked);
    connect(passwordEdit, &QLineEdit::returnPressed, this, &LoginDialog::onLoginClicked);

    usernameEdit->setFocus();
}

void LoginDialog::onLoginClicked()
{
    if (usernameEdit->text().trimmed().isEmpty()) {
        showError("Username cannot be empty");
        usernameEdit->setFocus();
        return;
    }

    if (passwordEdit->text().isEmpty()) {
        showError("Password cannot be empty");
        passwordEdit->setFocus();
        return;
    }

    clearError();
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
    statusLabel->setVisible(true);
}

void LoginDialog::clearError()
{
    statusLabel->setVisible(false);
}