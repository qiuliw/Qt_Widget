#include "widget.h"

#include <QApplication>


/*
    使用OpenGL的直接模式绘制视频

*/
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    qDebug() << "Application started";  // 检查是否输出
    w.show();
    return a.exec();
}
