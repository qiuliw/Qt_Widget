#include "xvideowidget.h"

#include <QOpenGLShaderProgram>
#include <algorithm>
#include <cstdio>
#include <gl/gl.h>
#include <qdebug.h>
#include <qopenglext.h>
#include <iostream>

// 自动加双引号
#define GET_STR(x) #x
#define A_VER 3
#define T_VER 4

FILE *fp = NULL;


// 顶点shader
const char* vString = GET_STR(
    attribute vec4 vertexIn;    // 输入：顶点坐标（XYZW）
    attribute vec2 textureIn;   // 输入：纹理坐标（UV）
    varying vec2 textureOut;    // 输出：传递给片段着色器的纹理坐标。会自动插值

    void main() {
        gl_Position = vertexIn; // 直接使用顶点坐标（无变换）
        textureOut = textureIn; // 传递纹理坐标
    }
);

// 片元shader
const char* tString = GET_STR(
    varying highp vec2 textureOut; // 输入：插值后的纹理坐标
    uniform sampler2D tex_y;       // Y 分量纹理
    uniform sampler2D tex_u;       // U 分量纹理
    uniform sampler2D tex_v;       // V 分量纹理

    void main() {
        vec3 yuv;
        vec3 rgb;
        // 从 YUV 纹理采样
        yuv.x = texture2D(tex_y, textureOut).r;      // Y分量：0.0~1.0
        yuv.y = texture2D(tex_u, textureOut).r - 0.5; // U分量归一化：-0.5~0.5
        yuv.z = texture2D(tex_v, textureOut).r - 0.5; // V分量归一化：-0.5~0.5
        // YUV 转 RGB（BT.601 标准）
        rgb = mat3(
            1.0,   1.0,    1.0,
            0.0,   -0.39465, 2.03211,
            1.13983, -0.58060, 0.0
        ) * yuv;
        gl_FragColor = vec4(rgb, 1.0); // 输出 RGBA 颜色
    }
);

XVideoWidget::XVideoWidget(QWidget *parent) : QOpenGLWidget(parent)
{
    // 初始化 program_ 指针
    program_ = new QOpenGLShaderProgram(this);
}

XVideoWidget::~XVideoWidget()
{
    // 释放 program_ 指针（由 Qt 的父子关系自动管理内存，可不写）
}

void XVideoWidget::initializeGL()
{
    initializeOpenGLFunctions();
    
    // 设置清屏颜色
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    // 使用立即模式CPU直接传递顶点和材质坐标，不先缓存到显存
    static const GLfloat verts[] = { // 顶点
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f,  1.0f,
    };
    static const GLfloat texCoords[] = { // 材质
        0.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
    };
    // 顶点属性
    glVertexAttribPointer(A_VER, 2, GL_FLOAT, 0, 0, verts);
    glEnableVertexAttribArray(A_VER);
    
    // 材质属性
    glVertexAttribPointer(T_VER, 2, GL_FLOAT, 0, 0, texCoords); 
    glEnableVertexAttribArray(T_VER);

    // 加载 shader
    program_->addShaderFromSourceCode(QOpenGLShader::Fragment, tString);
    program_->addShaderFromSourceCode(QOpenGLShader::Vertex, vString);
    
    // 关联shader变量与顶点属性索引。顶点着色器会从各个顶点属性索引中取1个顶点数据作为一组属性执行一次顶点着色器
    program_->bindAttributeLocation("vertexIn", A_VER);
    program_->bindAttributeLocation("textureIn", T_VER);

    // 编译并绑定 shader 程序
    qDebug() << "program_->link() =" << program_->link();
    qDebug() << "program_->bind() =" << program_->bind();

    // 获取shader材质索引
    unis[0] = program_->uniformLocation("tex_y");
    unis[1] = program_->uniformLocation("tex_u"); 
    unis[2] = program_->uniformLocation("tex_v");

    // 创建材质索引
    glGenTextures(3, texs); // 创建3个材质
    
    // Y
    glBindTexture(GL_TEXTURE_2D,texs[0]);
    // 放大缩小过滤，线性插值
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // 创建材质空间
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width_, height_, 0, GL_RED, GL_UNSIGNED_BYTE, 0);

    // U
    glBindTexture(GL_TEXTURE_2D,texs[1]);
    // 放大缩小过滤，线性插值
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // 创建材质空间
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width_/2, height_/2, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
    
    // V
    glBindTexture(GL_TEXTURE_2D,texs[2]);
    // 放大缩小过滤，线性插值
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // 创建材质空间
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width_/2, height_/2, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
    
    // 材质的内存空间
    datas[0] = new unsigned char[width_ * height_];
    datas[1] = new unsigned char[width_/2 * height_/2];
    datas[2] = new unsigned char[width_/2 * height_/2];

    
    fp = fopen("out240x128.yuv", "rb");
    if (fp == NULL) {
        qDebug() << "open file error";
        return;
    }
    
}

// 刷新显示
void XVideoWidget::paintGL()
{
    if(feof(fp))
    {
        fseek(fp, 0, SEEK_SET);
    }

    fread(datas[0], 1, width_ * height_, fp);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texs[0]); // 0层绑定到材质
    // 修改材质内容（复制内存内容到显存）
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width_, height_, GL_RED, GL_UNSIGNED_BYTE, datas[0]);
    // 与shader uni变量关联
    glUniform1i(unis[0],0);

    fread(datas[1], 1, width_/2 * height_/2, fp);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texs[1]); // 1层绑定到材质
    // 修改材质内容（复制内存内容到显存）
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width_/2, height_/2, GL_RED, GL_UNSIGNED_BYTE, datas[1]);
    // 与shader uni变量关联
    glUniform1i(unis[1],1);

    fread(datas[2], 1, width_/2 * height_/2, fp);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texs[2]); // 2层绑定到材质
    // 修改材质内容（复制内存内容到显存）
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width_/2, height_/2, GL_RED, GL_UNSIGNED_BYTE, datas[2]);
    // 与shader uni变量关联
    glUniform1i(unis[2],2);


    // 清空缓冲区
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    std::cout << "printGL()" << std::endl;
}

void XVideoWidget::resizeGL(int w, int h)
{
    // 设置视口
    glViewport(0, 0, w, h);
    std::cout << "resizeGL()" << std::endl;
}