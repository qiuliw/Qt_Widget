#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "XDemux.h"

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
private:
};
#endif // WIDGET_H
