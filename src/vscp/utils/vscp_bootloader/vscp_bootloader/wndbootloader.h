#ifndef WNDBOOTLOADER_H
#define WNDBOOTLOADER_H

#include <QMainWindow>

namespace Ui {
class wndBootloader;
}

class wndBootloader : public QMainWindow
{
    Q_OBJECT

public:
    explicit wndBootloader(QWidget *parent = 0);
    ~wndBootloader();

private:
    Ui::wndBootloader *ui;
};

#endif // WNDBOOTLOADER_H
