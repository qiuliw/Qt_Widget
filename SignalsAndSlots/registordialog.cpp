#include "registordialog.h"
#include <QPushButton>

RegistorDialog::RegistorDialog(QWidget *parent):QDialog(parent)
{
    this->setWindowTitle("这是注册窗口");
    this->setFixedSize(500,450);
    backBtn = new QPushButton("退出",this);
    backBtn->move(400,350);
    connect(backBtn,&QPushButton::clicked,[=](){
        emit registorSignal();
    });
}

