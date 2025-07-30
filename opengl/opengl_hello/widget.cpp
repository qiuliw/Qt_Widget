#include "widget.h"
#include "XVideoWidget.h"
#include <qboxlayout.h>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{

    resize(640, 480);

    // 创建垂直布局
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    // 设置布局边距为0，这样OpenGL窗口会填满整个窗口
    layout->setContentsMargins(0, 0, 0, 0);
    
    // 创建XVideoWidget实例
    videoWidget = new XVideoWidget(this);
    
    // 将XVideoWidget添加到布局中
    layout->addWidget(videoWidget);
    
    // 设置此布局为Widget的布局
    setLayout(layout);
}

Widget::~Widget()
{

}
