#include "widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    this->setWindowTitle("登录窗口");
    this->setFixedSize(500,300);

    title = new QLabel("用户请输入",this);
    userNameLineEdit = new QLineEdit(this);
    userNameLineEdit->setPlaceholderText("请输入用户名");
    pwdLineEdit = new QLineEdit(this);
    pwdLineEdit->setEchoMode(QLineEdit::Password);
    pwdLineEdit->setPlaceholderText("请输入密码");
    btnLogin = new QPushButton("登录",this);
    btnRegist = new QPushButton("注册",this);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    mainLayout->addWidget(title);
    mainLayout->addWidget(userNameLineEdit);
    mainLayout->addWidget(pwdLineEdit);
    // 设置居中，对象，对齐方式，没找到对象返回false
    mainLayout->setAlignment(title,Qt::AlignCenter);

    QHBoxLayout* secondLayout = new QHBoxLayout();
    secondLayout->addWidget(btnLogin);
    secondLayout->addWidget(btnRegist);
    mainLayout->addLayout(secondLayout);
}

Widget::~Widget() {}
