#include "widget.h"
#include <QPushButton>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    QPushButton* btn = new QPushButton(this);
    btn->children();

}

Widget::~Widget() {}
