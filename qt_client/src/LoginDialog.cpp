#include "LoginDialog.h"
#include <QTimer>

LoginDialog::LoginDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Messenger - Login");
    resize(900, 650);

    // Стилизация окна (CSS-подобные правила)
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

    // Основной вертикальный layout с отступами
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(40, 40, 40, 40);

    // Заголовок окна
    QLabel* titleLabel = new QLabel("Messenger");
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // Подзаголовок с приветствием
    QLabel* subtitleLabel = new QLabel("Welcome back! Please login or register");
    subtitleLabel->setStyleSheet("color: #8e6eb0; font-size: 12px; margin-bottom: 20px;");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(subtitleLabel);

    // Поле для ввода IP-адреса сервера (по умолчанию localhost)
    QLabel* serverLabel = new QLabel("Server IP:");
    serverIpEdit = new QLineEdit();
    serverIpEdit->setPlaceholderText("127.0.0.1");
    serverIpEdit->setText("127.0.0.1");
    mainLayout->addWidget(serverLabel);
    mainLayout->addWidget(serverIpEdit);

    // Поле для ввода имени пользователя
    QLabel* usernameLabel = new QLabel("Username:");
    usernameEdit = new QLineEdit();
    usernameEdit->setPlaceholderText("Enter your username");
    mainLayout->addWidget(usernameLabel);
    mainLayout->addWidget(usernameEdit);

    // Поле для ввода пароля (скрытый ввод)
    QLabel* passwordLabel = new QLabel("Password:");
    passwordEdit = new QLineEdit();
    passwordEdit->setPlaceholderText("Enter your password");
    passwordEdit->setEchoMode(QLineEdit::Password);  // Скрываем символы
    mainLayout->addWidget(passwordLabel);
    mainLayout->addWidget(passwordEdit);

    // Метка для отображения сообщений об ошибках (изначально скрыта)
    statusLabel = new QLabel();
    statusLabel->setObjectName("statusLabel");
    statusLabel->setVisible(false);
    statusLabel->setWordWrap(true);
    mainLayout->addWidget(statusLabel);

    // Блок кнопок
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    cancelButton = new QPushButton("Cancel");
    cancelButton->setObjectName("cancelButton");
    loginButton = new QPushButton("Login / Register");

    // Отключаем автоматическое поведение Enter, чтобы окно не закрывалось до проверки
    loginButton->setDefault(false);
    loginButton->setAutoDefault(false);
    cancelButton->setDefault(false);
    cancelButton->setAutoDefault(false);

    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(loginButton);
    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);

    // Подключение сигналов кнопок
    connect(loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    connect(cancelButton, &QPushButton::clicked, this, &LoginDialog::onCancelClicked);

    // Обработка нажатия Enter в полях username и password
    // QTimer::singleShot(0, ...) нужен, чтобы окно не закрывалось до выполнения проверки
    connect(usernameEdit, &QLineEdit::returnPressed, this, [this]() {
        QTimer::singleShot(0, this, &LoginDialog::onLoginClicked);
        });
    connect(passwordEdit, &QLineEdit::returnPressed, this, [this]() {
        QTimer::singleShot(0, this, &LoginDialog::onLoginClicked);
        });

    usernameEdit->setFocus();  // Устанавливаем фокус на поле имени
}

// Обработчик нажатия кнопки Login / Register
void LoginDialog::onLoginClicked()
{
    QString username = usernameEdit->text().trimmed();  // Убираем лишние пробелы
    QString password = passwordEdit->text();

    // Проверка: имя не должно быть пустым
    if (username.isEmpty()) {
        showError("Username cannot be empty");
        usernameEdit->setFocus();
        return;
    }

    // Проверка: пароль не должен быть пустым
    if (password.isEmpty()) {
        showError("Password cannot be empty");
        passwordEdit->setFocus();
        return;
    }

    // Все проверки пройдены — закрываем диалог с кодом Accepted
    accept();
}

// Обработчик нажатия кнопки Cancel
void LoginDialog::onCancelClicked()
{
    reject();  // Закрываем диалог с кодом Rejected
}

// Возвращает введённый IP-адрес сервера
QString LoginDialog::getServerIp() const
{
    return serverIpEdit->text().trimmed();
}

// Возвращает введённое имя пользователя
QString LoginDialog::getUsername() const
{
    return usernameEdit->text().trimmed();
}

// Возвращает введённый пароль
QString LoginDialog::getPassword() const
{
    return passwordEdit->text();
}

// Показывает сообщение об ошибке (красная метка)
void LoginDialog::showError(const QString& message)
{
    statusLabel->setText(message);
    statusLabel->show();
    statusLabel->raise();   // Поднимаем на передний план
    statusLabel->update();  // Принудительное обновление

    // Автоматически скрыть сообщение через 3 секунды
    QTimer::singleShot(3000, this, [this]() {
        if (statusLabel) {
            statusLabel->hide();
        }
        });
}

// Скрывает сообщение об ошибке
void LoginDialog::clearError()
{
    statusLabel->hide();
}