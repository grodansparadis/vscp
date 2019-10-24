/********************************************************************************
** Form generated from reading UI file 'wndbootloader.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WNDBOOTLOADER_H
#define UI_WNDBOOTLOADER_H

#include <QtCore/QLocale>
#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_wndBootloader
{
public:
    QWidget *centralWidget;
    QLabel *labelLogo;
    QPushButton *pushButtonHelp;
    QPushButton *pushButtonPrev;
    QPushButton *pushButtonNext;
    QPushButton *pushButtonCancel;
    QLabel *label_2;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;

    void setupUi(QMainWindow *wndBootloader)
    {
        if (wndBootloader->objectName().isEmpty())
            wndBootloader->setObjectName(QStringLiteral("wndBootloader"));
        wndBootloader->resize(668, 442);
        wndBootloader->setAutoFillBackground(false);
        wndBootloader->setStyleSheet(QStringLiteral(""));
        centralWidget = new QWidget(wndBootloader);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        labelLogo = new QLabel(centralWidget);
        labelLogo->setObjectName(QStringLiteral("labelLogo"));
        labelLogo->setGeometry(QRect(20, 10, 231, 131));
        labelLogo->setAutoFillBackground(false);
        labelLogo->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        labelLogo->setFrameShape(QFrame::NoFrame);
        labelLogo->setFrameShadow(QFrame::Plain);
        labelLogo->setPixmap(QPixmap(QString::fromUtf8(":/new/pictures/resources/vscp_new.png")));
        labelLogo->setScaledContents(true);
        labelLogo->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        pushButtonHelp = new QPushButton(centralWidget);
        pushButtonHelp->setObjectName(QStringLiteral("pushButtonHelp"));
        pushButtonHelp->setGeometry(QRect(280, 370, 75, 31));
        pushButtonPrev = new QPushButton(centralWidget);
        pushButtonPrev->setObjectName(QStringLiteral("pushButtonPrev"));
        pushButtonPrev->setEnabled(false);
        pushButtonPrev->setGeometry(QRect(390, 370, 75, 31));
        pushButtonNext = new QPushButton(centralWidget);
        pushButtonNext->setObjectName(QStringLiteral("pushButtonNext"));
        pushButtonNext->setGeometry(QRect(470, 370, 75, 31));
        pushButtonCancel = new QPushButton(centralWidget);
        pushButtonCancel->setObjectName(QStringLiteral("pushButtonCancel"));
        pushButtonCancel->setGeometry(QRect(570, 370, 75, 31));
        label_2 = new QLabel(centralWidget);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(350, 40, 301, 261));
        label_2->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        label_2->setTextFormat(Qt::AutoText);
        label_2->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        label_2->setWordWrap(true);
        wndBootloader->setCentralWidget(centralWidget);
        pushButtonHelp->raise();
        pushButtonPrev->raise();
        pushButtonNext->raise();
        pushButtonCancel->raise();
        label_2->raise();
        labelLogo->raise();
        menuBar = new QMenuBar(wndBootloader);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 668, 21));
        wndBootloader->setMenuBar(menuBar);
        mainToolBar = new QToolBar(wndBootloader);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        wndBootloader->addToolBar(Qt::TopToolBarArea, mainToolBar);

        retranslateUi(wndBootloader);

        QMetaObject::connectSlotsByName(wndBootloader);
    } // setupUi

    void retranslateUi(QMainWindow *wndBootloader)
    {
        wndBootloader->setWindowTitle(QApplication::translate("wndBootloader", "VSCP Bootloader", 0));
        labelLogo->setText(QString());
        pushButtonHelp->setText(QApplication::translate("wndBootloader", "Help", 0));
        pushButtonPrev->setText(QApplication::translate("wndBootloader", "< Prev", 0));
        pushButtonNext->setText(QApplication::translate("wndBootloader", "Next >", 0));
        pushButtonCancel->setText(QApplication::translate("wndBootloader", "Cancel", 0));
        label_2->setText(QApplication::translate("wndBootloader", "VSCP Bootloader wizard will now walk you through \n"
"the steps needed to update he firmware of your \n"
"remote device. \n"
"\n"
"The node you need to update can be located on a VSCP bus \n"
"connected to a server on a remote location or be connected\n"
"to an interface on your local computer. \n"
"\n"
"Please don't turn of the power off the remote node \n"
"until the firmware update process is completed.", 0));
    } // retranslateUi

};

namespace Ui {
    class wndBootloader: public Ui_wndBootloader {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WNDBOOTLOADER_H
