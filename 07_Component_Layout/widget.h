#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QDialog>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>

class Widget : public QWidget
{
    Q_OBJECT

private:
    QLabel* title;
    QLineEdit* userNameLineEdit;
    QLineEdit* pwdLineEdit;
    QPushButton* btnLogin;
    QPushButton* btnRegist;

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
};
#endif // WIDGET_H
