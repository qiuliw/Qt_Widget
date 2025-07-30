#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>


/*
 * 使用纯C++实现Qt对象树内存回收机制底层原理
 */
class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
};
#endif // WIDGET_H
