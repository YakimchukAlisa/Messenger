#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

// Диалоговое окно для входа в мессенджер
// Запрашивает IP сервера, имя пользователя и пароль
// При нажатии Login отправляет сигнал accept(), при Cancel - reject()
class LoginDialog : public QDialog
{
    Q_OBJECT

private:
    // Поля ввода
    QLineEdit* serverIpEdit;    // IP-адрес сервера (по умолчанию 127.0.0.1)
    QLineEdit* usernameEdit;    // Имя пользователя
    QLineEdit* passwordEdit;    // Пароль (скрытый ввод)

    // Кнопки
    QPushButton* loginButton;   // Кнопка входа/регистрации
    QPushButton* cancelButton;  // Кнопка отмены

    QLabel* statusLabel;        // Метка для отображения ошибок (красный текст)

public:
    explicit LoginDialog(QWidget* parent = nullptr);

    // Геттеры для получения введённых данных
    QString getServerIp() const;    // Возвращает IP сервера
    QString getUsername() const;    // Возвращает имя пользователя
    QString getPassword() const;    // Возвращает пароль

    // Управление сообщениями об ошибках
    void showError(const QString& message);  // Показать ошибку
    void clearError();                       // Скрыть ошибку

private slots:
    void onLoginClicked();   // Обработчик нажатия кнопки Login
    void onCancelClicked();  // Обработчик нажатия кнопки Cancel
};

#endif