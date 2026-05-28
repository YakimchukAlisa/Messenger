#include <QApplication>
#include "LoginDialog.h"
#include "MainWindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // Сначала показываем окно логина
    LoginDialog loginDialog;
    if (loginDialog.exec() != QDialog::Accepted) {
        return 0;  // Выходим без запуска главного окна
    }

    // Получаем данные из логина
    QString serverIP = loginDialog.getServerIp();
    QString username = loginDialog.getUsername();
    QString password = loginDialog.getPassword();

    // Создаём главное окно и передаём данные
    MainWindow window(serverIP, username, password);
    window.show();

    return app.exec();
}