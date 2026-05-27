#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

class LoginDialog : public QDialog
{
    Q_OBJECT

private:
    QLineEdit* serverIpEdit;
    QLineEdit* usernameEdit;
    QLineEdit* passwordEdit;
    QPushButton* loginButton;
    QPushButton* cancelButton;
    QLabel* statusLabel;

public:
    LoginDialog(QWidget* parent = nullptr);

    QString getServerIp() const;
    QString getUsername() const;
    QString getPassword() const;

    void showError(const QString& message);
    void clearError();

private slots:
    void onLoginClicked();
    void onCancelClicked();
};

#endif