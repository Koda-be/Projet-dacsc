#include "mainwindowclientbookencoder.h"

#include <fstream>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    std::ifstream config{".client.cfg"};
    std::string ip{};
    int port{};
    config >> ip;
    config >> port;
    MainWindowClientBookEncoder w{ip, port};
    w.show();
    return a.exec();
}
