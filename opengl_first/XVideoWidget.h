#pragma once

#include <QtOpenGLWidgets/QtOpenGLWidgets>

class XVideoWidget : public QOpenGLWidget,protected QOpenGLFunctions
{
    Q_OBJECT

public:
    XVideoWidget(QWidget *parent = nullptr);

protected:
    void initializeGL() override; // 初始化
    void paintGL() override; // 绘制
    void resizeGL(int w, int h) override; // 改变大小
};