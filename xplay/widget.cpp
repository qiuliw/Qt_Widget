#include "widget.h"
#include "XVideoWidget.h"
#include <QVBoxLayout>  // 用于布局管理

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{

    resize(800, 600);

    // 创建 XVideoWidget
    videoWidget = new XVideoWidget(this);
    videoWidget->setFixedSize(800, 600);  // 固定大小 800x600

    // 创建布局管理器，使 videoWidget 居中
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addStretch();  // 上方弹性空间
    layout->addWidget(videoWidget, 0, Qt::AlignCenter);  // 居中
    layout->addStretch();  // 下方弹性空间

    // 设置布局
    this->setLayout(layout);
}

Widget::~Widget() {
    // 析构函数（如有需要，可添加清理代码）
}