#pragma once

#include <QtOpenGL/qopenglshaderprogram.h>
#include <QtOpenGLWidgets/QOpenGLWidget>
#include <QOpenGLFunctions>

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

};
