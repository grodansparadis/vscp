/********************************************************************************
** Form generated from reading UI file 'wndbootloader.ui'
**
** Created by: Qt User Interface Compiler version 5.2.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WNDBOOTLOADER_H
#define UI_WNDBOOTLOADER_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_wndBootloader
{
public:
    QWidget *centralWidget;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *wndBootloader)
    {
        if (wndBootloader->objectName().isEmpty())
            wndBootloader->setObjectName(QStringLiteral("wndBootloader"));
        wndBootloader->resize(558, 422);
        centralWidget = new QWidget(wndBootloader);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        wndBootloader->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(wndBootloader);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 558, 21));
        wndBootloader->setMenuBar(menuBar);
        mainToolBar = new QToolBar(wndBootloader);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        wndBootloader->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(wndBootloader);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        wndBootloader->setStatusBar(statusBar);

        retranslateUi(wndBootloader);

        QMetaObject::connectSlotsByName(wndBootloader);
    } // setupUi

    void retranslateUi(QMainWindow *wndBootloader)
    {
        wndBootloader->setWindowTitle(QApplication::translate("wndBootloader", "wndBootloader", 0));
    } // retranslateUi

};

namespace Ui {
    class wndBootloader: public Ui_wndBootloader {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WNDBOOTLOADER_H
