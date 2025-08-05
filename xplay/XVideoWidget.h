#pragma once

#include "IVideoCall.h"
#include <QtOpenGL/qopenglshaderprogram.h>
#include <QtOpenGLWidgets/QOpenGLWidget>
#include <QOpenGLFunctions>
#include <mutex>

class AVFrame;

class XVideoWidget : public QOpenGLWidget, protected QOpenGLFunctions, public IVideoCall
{
    Q_OBJECT

public:
    XVideoWidget(QWidget *parent = nullptr);
    ~XVideoWidget();
    // 初始化材质内存与材质原始宽高，每次渲染时从frame读入材质
    void Init(int width, int height) override;
    // 不管成功与否都释放掉frame空间
    virtual void Repaint(AVFrame *frame) override;// 画面重绘
    int Height(){return height_;}
    int Width(){return width_;}
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
    unsigned char* datas[3] = {0};

    int width_ = 600;
    int height_ = 480;

    std::mutex mtx_;
};
