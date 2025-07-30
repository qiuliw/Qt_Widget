#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

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
    void Refresh();
    // 用户选择一个文件，插入到列表
    void Upload();

private:
    Ui::Widget *ui;
};
#endif // WIDGET_H
