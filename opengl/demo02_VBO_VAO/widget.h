#ifndef WIDGET_H
#define WIDGET_H

#include "XVideoWidget.h"
#include <QWidget>

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    XVideoWidget *videoWidget;
};
#endif // WIDGET_H
