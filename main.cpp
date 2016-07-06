#include "mainwindow.h"
#include <QApplication>

//PROSWRINA COMMENTED GIA DOKIMH
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    a.setApplicationName("USB Receiver");
    //QCoreApplication::setApplicationName(APPLICATION_NAME);
    //QCoreApplication::setOrganizationName(ORGANIZATION_NAME);

    w.show();
    
    return a.exec();
}
