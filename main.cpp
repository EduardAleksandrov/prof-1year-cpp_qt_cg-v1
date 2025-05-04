#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
//    w.setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
//    w.showFullScreen();
    // Установка окна в максимизированный режим
    w.showMaximized();
    w.show();
    return a.exec();
}
