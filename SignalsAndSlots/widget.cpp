#include "widget.h"
#include "./ui_widget.h"
#include <QPushButton>
#include <QDebug>
#include "registordialog.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->setFixedSize(500,800);
    this->setWindowTitle("控件练习");
    btnLogin = new QPushButton("登录",this);
    btnRegis = new QPushButton("注册",this);
    btnQuite = new QPushButton("退出",this);

    btnLogin->move(100,50);
    btnRegis->move(200,50);
    btnQuite->move(300,50);

    // 这种方式是Qt4中所用的方式
    //connect(btnQuite,SIGNAL(clicked(bool)),this,SLOT(close()));
    // Qt5以上所用的方式
    //connect(btnQuite,&QPushButton::clicked,this,&QWidget::close);
    // Lambda表达式
    connect(btnQuite,&QPushButton::clicked,[=](){
        this->close();
    });

    connect(btnRegis,&QPushButton::clicked,this,&Widget::openRegistorDialog);

    regisDialog = new RegistorDialog(this);

    //connect(regisDialog->getBackBtn(),&QPushButton::clicked,regisDialog,&RegistorDialog::close);

    connect(regisDialog,&RegistorDialog::registorSignal,[=](){
        regisDialog->hide();
    });
}

Widget::~Widget()
{
    delete ui;
}

void Widget::openRegistorDialog()
{
    regisDialog->open();
    qDebug() << "登录窗口被打开";
}
