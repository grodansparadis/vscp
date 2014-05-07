/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionClient_Window;
    QAction *actionDevice_Configuration_Window;
    QAction *actionMDF_Editor;
    QAction *actionDecision_Matrix_Editor_daemon;
    QAction *actionVariable_Editor;
    QAction *actionScan_for_devices;
    QAction *actionBoot_loader_wizard;
    QAction *actionSimple_UI_Designer;
    QAction *actionMerlin_Interface_Designer;
    QAction *actionAbout;
    QAction *actionConfiguration;
    QAction *actionMDF_Editor_2;
    QAction *actionVSCP_Works_Help;
    QAction *actionFrquently_Asked_Questions;
    QAction *actionKeyboard_Shortcuts;
    QAction *actionThanks;
    QAction *actionCredits;
    QAction *actionGoto_VSCP_Site;
    QWidget *centralWidget;
    QGraphicsView *graphicsView;
    QMenuBar *menuBar;
    QMenu *menuVSCP;
    QMenu *menuTools;
    QMenu *menuHelp;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(676, 544);
        actionClient_Window = new QAction(MainWindow);
        actionClient_Window->setObjectName(QStringLiteral("actionClient_Window"));
        actionDevice_Configuration_Window = new QAction(MainWindow);
        actionDevice_Configuration_Window->setObjectName(QStringLiteral("actionDevice_Configuration_Window"));
        actionMDF_Editor = new QAction(MainWindow);
        actionMDF_Editor->setObjectName(QStringLiteral("actionMDF_Editor"));
        actionDecision_Matrix_Editor_daemon = new QAction(MainWindow);
        actionDecision_Matrix_Editor_daemon->setObjectName(QStringLiteral("actionDecision_Matrix_Editor_daemon"));
        actionVariable_Editor = new QAction(MainWindow);
        actionVariable_Editor->setObjectName(QStringLiteral("actionVariable_Editor"));
        actionScan_for_devices = new QAction(MainWindow);
        actionScan_for_devices->setObjectName(QStringLiteral("actionScan_for_devices"));
        actionBoot_loader_wizard = new QAction(MainWindow);
        actionBoot_loader_wizard->setObjectName(QStringLiteral("actionBoot_loader_wizard"));
        actionSimple_UI_Designer = new QAction(MainWindow);
        actionSimple_UI_Designer->setObjectName(QStringLiteral("actionSimple_UI_Designer"));
        actionMerlin_Interface_Designer = new QAction(MainWindow);
        actionMerlin_Interface_Designer->setObjectName(QStringLiteral("actionMerlin_Interface_Designer"));
        actionAbout = new QAction(MainWindow);
        actionAbout->setObjectName(QStringLiteral("actionAbout"));
        actionConfiguration = new QAction(MainWindow);
        actionConfiguration->setObjectName(QStringLiteral("actionConfiguration"));
        actionMDF_Editor_2 = new QAction(MainWindow);
        actionMDF_Editor_2->setObjectName(QStringLiteral("actionMDF_Editor_2"));
        actionVSCP_Works_Help = new QAction(MainWindow);
        actionVSCP_Works_Help->setObjectName(QStringLiteral("actionVSCP_Works_Help"));
        actionFrquently_Asked_Questions = new QAction(MainWindow);
        actionFrquently_Asked_Questions->setObjectName(QStringLiteral("actionFrquently_Asked_Questions"));
        actionKeyboard_Shortcuts = new QAction(MainWindow);
        actionKeyboard_Shortcuts->setObjectName(QStringLiteral("actionKeyboard_Shortcuts"));
        actionThanks = new QAction(MainWindow);
        actionThanks->setObjectName(QStringLiteral("actionThanks"));
        actionCredits = new QAction(MainWindow);
        actionCredits->setObjectName(QStringLiteral("actionCredits"));
        actionGoto_VSCP_Site = new QAction(MainWindow);
        actionGoto_VSCP_Site->setObjectName(QStringLiteral("actionGoto_VSCP_Site"));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        graphicsView = new QGraphicsView(centralWidget);
        graphicsView->setObjectName(QStringLiteral("graphicsView"));
        graphicsView->setGeometry(QRect(100, 30, 481, 371));
        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 676, 26));
        menuVSCP = new QMenu(menuBar);
        menuVSCP->setObjectName(QStringLiteral("menuVSCP"));
        menuTools = new QMenu(menuBar);
        menuTools->setObjectName(QStringLiteral("menuTools"));
        menuHelp = new QMenu(menuBar);
        menuHelp->setObjectName(QStringLiteral("menuHelp"));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MainWindow->setStatusBar(statusBar);

        menuBar->addAction(menuVSCP->menuAction());
        menuBar->addAction(menuTools->menuAction());
        menuBar->addAction(menuHelp->menuAction());
        menuVSCP->addAction(actionClient_Window);
        menuVSCP->addAction(actionDevice_Configuration_Window);
        menuVSCP->addAction(actionMDF_Editor);
        menuVSCP->addSeparator();
        menuVSCP->addAction(actionDecision_Matrix_Editor_daemon);
        menuVSCP->addAction(actionVariable_Editor);
        menuVSCP->addSeparator();
        menuVSCP->addAction(actionScan_for_devices);
        menuVSCP->addAction(actionBoot_loader_wizard);
        menuVSCP->addSeparator();
        menuVSCP->addAction(actionSimple_UI_Designer);
        menuVSCP->addAction(actionMerlin_Interface_Designer);
        menuTools->addAction(actionConfiguration);
        menuTools->addAction(actionMDF_Editor_2);
        menuHelp->addAction(actionVSCP_Works_Help);
        menuHelp->addAction(actionFrquently_Asked_Questions);
        menuHelp->addAction(actionKeyboard_Shortcuts);
        menuHelp->addAction(actionThanks);
        menuHelp->addAction(actionCredits);
        menuHelp->addAction(actionGoto_VSCP_Site);
        menuHelp->addAction(actionAbout);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
        actionClient_Window->setText(QApplication::translate("MainWindow", "Client Window...", 0));
        actionDevice_Configuration_Window->setText(QApplication::translate("MainWindow", "Device Configuration Window", 0));
        actionMDF_Editor->setText(QApplication::translate("MainWindow", "MDF Editor", 0));
        actionDecision_Matrix_Editor_daemon->setText(QApplication::translate("MainWindow", "Decision Matrix Editor (daemon)...", 0));
        actionVariable_Editor->setText(QApplication::translate("MainWindow", "Variable Editor....", 0));
        actionScan_for_devices->setText(QApplication::translate("MainWindow", "Scan for devices...", 0));
        actionBoot_loader_wizard->setText(QApplication::translate("MainWindow", "Boot loader wizard...", 0));
        actionSimple_UI_Designer->setText(QApplication::translate("MainWindow", "Simple UI Designer...", 0));
        actionMerlin_Interface_Designer->setText(QApplication::translate("MainWindow", "Merlin Interface Designer...", 0));
        actionAbout->setText(QApplication::translate("MainWindow", "About", 0));
        actionConfiguration->setText(QApplication::translate("MainWindow", "Configuration...", 0));
        actionMDF_Editor_2->setText(QApplication::translate("MainWindow", "MDF Editor...", 0));
        actionVSCP_Works_Help->setText(QApplication::translate("MainWindow", "VSCP Works Help", 0));
        actionFrquently_Asked_Questions->setText(QApplication::translate("MainWindow", "Frquently Asked Questions", 0));
        actionKeyboard_Shortcuts->setText(QApplication::translate("MainWindow", "Keyboard Shortcuts", 0));
        actionThanks->setText(QApplication::translate("MainWindow", "Thanks...", 0));
        actionCredits->setText(QApplication::translate("MainWindow", "Credits...", 0));
        actionGoto_VSCP_Site->setText(QApplication::translate("MainWindow", "Goto VSCP Site", 0));
        menuVSCP->setTitle(QApplication::translate("MainWindow", "VSCP", 0));
        menuTools->setTitle(QApplication::translate("MainWindow", "Tools", 0));
        menuHelp->setTitle(QApplication::translate("MainWindow", "Help", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
