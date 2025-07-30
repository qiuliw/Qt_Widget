#include "mainwindow.h"
#include <qkeysequence.h>
#include <qmenubar.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->setWindowTitle("第一个可编辑文本软件主界面");
    this->setFixedSize(1024,768);
    
    initMenuBar();
}

MainWindow::~MainWindow() {}


void MainWindow::initMenuBar()
{
    mainMenuBar = new QMenuBar; // 这里可以不设置父对象
    this->setMenuBar(mainMenuBar); // add set隐式设定父对象
    // 选项
    fileMenu = mainMenuBar->addMenu("文件");
        // 动作
    newAction = fileMenu->addAction("新建文件");
    newAction->setIcon(QIcon(":/img/a.jpg"));
    newAction->setShortcut(QKeySequence("Ctrl+N"));

    // 分隔符
    fileMenu->addSeparator();

    openAction = fileMenu->addAction("关闭文件");
    saveAction = fileMenu->addAction("保存文件");
    
    editMenu = mainMenuBar->addMenu("编辑");
    buildMenu = mainMenuBar->addMenu("构建");
}

void MainWindow::initToolBar() {
    mainToolBar = this->addToolBar("文件相关");
    mainToolBar->addAction(newAction);//工具栏可以添加菜单栏中的选项对象
    mainToolBar->addAction(saveAction);
    mainToolBar->addSeparator();
}
