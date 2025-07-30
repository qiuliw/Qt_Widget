#pragma once

#include <QtOpenGL/qopenglshaderprogram.h>
#include <QtOpenGLWidgets/QOpenGLWidget>
#include <QOpenGLFunctions>
#include <gl/gl.h>

class XVideoWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    XVideoWidget(QWidget *parent = nullptr);
    ~XVideoWidget();

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

private:
    // shader程序
    QOpenGLShaderProgram *program_;
    // shader中yuv变量地址
    GLuint unis[3] = {0};
    // shader中texture变量地址
    GLuint texs[3] = {0};
    // 材质的内存空间
    std::unique_ptr<unsigned char[]> datas[3];

    int width_ = 240;
    int height_ = 128;
};
