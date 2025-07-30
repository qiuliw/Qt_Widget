#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QToolButton>
#include <QTextEdit>
#include <QMenuBar>
#include <QStatusBar>
#include <qaction.h>
#include <qmenu.h>
#include <qmenubar.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:

/*
    mainwindow
        // 一行
        QMenuBar
            QAction
            QAction
            QAction
        QMenuBar
            QAction
            QAction
            QAction
        QMenuBar
            QAction
            QAction
            QAction

        // 一行
        QToolBar
            QAction
            QToolButton
            QAction
            QToolButton
            QToolButton

*/

    QMenuBar* mainMenuBar;

    QMenu* fileMenu;
    QAction* newAction;
    QAction* openAction;
    QAction* saveAction;
    QMenu* editMenu;
    QMenu* buildMenu;

    QToolBar* mainToolBar;
    QToolButton* newButton;


    QTextEdit* mainEditWidget;
    QStatusBar* mainStatusBar;
    

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void initMenuBar();
    void initToolBar();
};
#endif // MAINWINDOW_H
