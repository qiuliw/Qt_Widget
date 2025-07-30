#include "widget.h"

#include <QApplication>

#include "./ui_widget.h"

#include <QRegularExpression>
#include <QMessageBox>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    connect(ui->userNameLine, &QLineEdit::textChanged, [=](const QString& str) {
        QRegularExpression reg("^[A-Z][a-z0-9_]{0,}$");
        QRegularExpressionMatch match = reg.match(str);

        if (match.hasMatch()) {
            // 用户名格式正确时的处理逻辑
            ui->userNameLine->setStyleSheet("border: 1px solid green;");
        } else {
            // 用户名格式错误时的处理逻辑
            ui->userNameLine->setStyleSheet("border: 1px solid red;");
        }
    });

    connect(ui->loginPushButton,&QPushButton::clicked,this,&Widget::on_btnLogin_clicked);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_btnLogin_clicked()
{
    QMessageBox::information(this,"提示","登录成功",
                             QMessageBox::Yes
        );
}
