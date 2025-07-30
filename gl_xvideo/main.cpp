#include "widget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    qDebug() << "Application started";  // 检查是否输出
    w.show();
    return a.exec();
}
