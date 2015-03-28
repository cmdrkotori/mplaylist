#include "window.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication::setOrganizationDomain("github.cmdrkotori.mplaylist");
    QApplication::setOrganizationName("mplaylist");
    QApplication::setApplicationName("mplaylist");
    QApplication a(argc, argv);
    Window w;
    w.show();

    return a.exec();
}
