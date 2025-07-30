#include "xvideowidget.h"

#include <QOpenGLShaderProgram>
#include <qdebug.h>
#include <qlogging.h>

// 自动加双引号
#define GET_STR(x) #x

XVideoWidget::XVideoWidget(QWidget *parent) : QOpenGLWidget(parent)
{
}

XVideoWidget::~XVideoWidget()
{
}

void XVideoWidget::initializeGL()
{
    initializeOpenGLFunctions(); // 初始化全局的OpenGL函数指针

    program_ = new QOpenGLShaderProgram(this); // 内部会直接调用全局的OpenGL函数指针
    
    
    
}

void XVideoWidget::paintGL()
{
    qDebug() << "paintGL()";
}

void XVideoWidget::resizeGL(int w, int h)
{
    qDebug() << "resizeGL():" << w << "," << h;

}