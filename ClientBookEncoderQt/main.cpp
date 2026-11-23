#include "mainwindowclientbookencoder.h"

#include <fstream>
#include <QApplication>
#include <QDir>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    std::ifstream config{".client.cfg"};
    std::string ip{};
    int port{};
    std::string reste;
    if (!(config >> ip >> port) || port < 1 || port > 65535 || (config >> reste))
    {
        QMessageBox::critical(nullptr, "Configuration",
                              "Adresse et port attendus dans " +
                                  QDir::current().absoluteFilePath(".client.cfg"));
        return 1;
    }
    MainWindowClientBookEncoder w{ip, port};
    w.show();
    return a.exec();
}
