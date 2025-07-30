#include "widget.h"
#include "./ui_widget.h"
#include "QMessageBox"
#include <qfiledialog.h>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::Refresh()
{
    QMessageBox::information(this,"","Refresh");

}

void Widget::Upload()
{
    QMessageBox::information(this,"","Upload");
    // 用户选择一个文件
    QString filename = QFileDialog::getOpenFileName(this,QString::fromLocal8Bit("请选择上传文件"));
    if(filename.isEmpty())
        return;

    // 插入到列表
    ui->fileListWidget->insertRow(0); // 插入在开头位置

    // 获取文件大小（字节） - 使用QFileInfo（推荐，无需打开文件）
    QFileInfo fileInfo(filename);
    qint64 fileSize = fileInfo.size(); // 直接获取文件大小（字节）

    // 设置文件名和大小到列表
    ui->fileListWidget->setItem(0, 0, new QTableWidgetItem(fileInfo.fileName())); // 文件名
    ui->fileListWidget->setItem(0, 1, new QTableWidgetItem(tr("%1 Byte").arg(fileSize))); // 文件大小

}
