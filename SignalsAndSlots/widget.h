#ifndef WIDGET_H
#define WIDGET_H

#include <QPushButton>
#include <QWidget>
#include "registordialog.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

public slots:
    void openRegistorDialog();


private:
    Ui::Widget *ui;

    QPushButton* btnLogin;
    QPushButton* btnRegis;
    QPushButton* btnQuite;

    RegistorDialog* regisDialog;

};
#endif // WIDGET_H
