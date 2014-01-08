#include "wndbootloader.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    wndBootloader w;
    w.show();

    return a.exec();
}
