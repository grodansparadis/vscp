#include "wndbootloader.h"
#include "ui_wndbootloader.h"

wndBootloader::wndBootloader(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::wndBootloader)
{
    ui->setupUi(this);
}

wndBootloader::~wndBootloader()
{
    delete ui;
}
