#include "xvideowidget.h"

#include <QOpenGLShaderProgram>
#include <algorithm>
#include <cstdio>
#include <mutex>
#include <ostream>
#include <qdebug.h>
#include <qobjectdefs.h>
#include <qopenglext.h>
#include <iostream>
#include <QTimer>
#include <QWidget>
#include <libavutil/frame.h>


// 自动加双引号
#define GET_STR(x) #x
#define A_VER 3
#define T_VER 4

FILE *fp = NULL; // 文件入口


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
    varying highp vec2 textureOut;
    uniform sampler2D tex_y;
    uniform sampler2D tex_u;
    uniform sampler2D tex_v;

    void main() {
        float y = texture2D(tex_y, textureOut).r;
        float u = texture2D(tex_u, textureOut).r;
        float v = texture2D(tex_v, textureOut).r;
        
        y = 1.1643 * (y - 0.0625);
        u = u - 0.5;
        v = v - 0.5;
        
        // YUV到RGB转换
        float r = y + 1.5958 * v;
        float g = y - 0.3917 * u - 0.8129 * v;
        float b = y + 2.017 * u;
        
        gl_FragColor = vec4(r, g, b, 1.0);
    }
);

XVideoWidget::XVideoWidget(QWidget *parent) : QOpenGLWidget(parent)
{
    // 初始化 program_ 指针
    program_ = new QOpenGLShaderProgram(this);
}

XVideoWidget::~XVideoWidget()
{

}

// 初始化材质内存与材质原始宽高，每次渲染时从文件读入材质
void XVideoWidget::Init(int width, int height)
{
    std::lock_guard<std::mutex> lk(mtx_);

    this->width_ = width;
    this->height_ = height;

    for(int i = 0; i < 3; i++){
        if(datas[i]){
            delete datas[i];
        }
    }
    // 材质的内存空间。宽高是文件的原始宽高
    datas[0] = new unsigned char[width_ * height_];
    datas[1] = new unsigned char[width_ * height_ / 4];
    datas[2] = new unsigned char[width_ * height_ / 4];
    
    if(texs[0])
    {
        glDeleteTextures(3, texs);
    }
    // 创建材质索引
    glGenTextures(3, texs); // 创建3个材质
    
    // Y
    glBindTexture(GL_TEXTURE_2D,texs[0]);
    // 放大缩小过滤，线性插值。GL_NEAREST(效率高，但马赛克严重)
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
    
}

// 将帧读入材质内存,绘制
void XVideoWidget::Repaint(AVFrame *frame)
{
    std::lock_guard<std::mutex> lk(mtx_);
    // 容错，保证尺寸正确。保证是视频帧
    if(!datas[0] || this->width_*height_ == 0 || frame->width != width_ || frame->height != height_){
        return;
    }


    // 检查像素格式是否为YUV420P
    if (frame->format != AV_PIX_FMT_YUV420P) {
        qDebug() << "Warning: Frame format is not YUV420P:" << frame->format;
        // 可能需要进行格式转换
    }

    // 行对齐问题
    // 修复行对齐问题 - 正确处理frame的linesize
    // Y平面
    for (int i = 0; i < height_; i++) {
        memcpy(datas[0] + i * width_, frame->data[0] + i * frame->linesize[0], width_);
    }
    
    // U平面
    for (int i = 0; i < height_/2; i++) {
        memcpy(datas[1] + i * width_/2, frame->data[1] + i * frame->linesize[1], width_/2);
    }
    
    // V平面
    for (int i = 0; i < height_/2; i++) {
        memcpy(datas[2] + i * width_/2, frame->data[2] + i * frame->linesize[2], width_/2);
    }
    // 刷新显示
    update();
}

void XVideoWidget::initializeGL()
{
    std::lock_guard<std::mutex> lk(mtx_);

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

    qDebug() << "材质数组";
    // 顶点属性
    glVertexAttribPointer(A_VER, 2, GL_FLOAT, 0, 0, verts);
    glEnableVertexAttribArray(A_VER);
    
    // 材质属性
    glVertexAttribPointer(T_VER, 2, GL_FLOAT, 0, 0, texCoords);
    glEnableVertexAttribArray(T_VER);

    // 加载 shader
    if (!program_->addShaderFromSourceCode(QOpenGLShader::Vertex, vString)) {
        qDebug() << "Vertex shader error:" << program_->log();
    }
    if (!program_->addShaderFromSourceCode(QOpenGLShader::Fragment, tString)) {
        qDebug() << "Fragment shader error:" << program_->log();
    }
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

//     fp = fopen("out240x128.yuv", "rb");
//     if (fp == NULL) {
//         qDebug() << "open file error";
//         return;
//     }

//     // 启动定时器
//     QTimer *timer = new QTimer(this);
//     // QOverload<>::of 是 Qt 提供的一个用于解决 C++ 函数重载时信号与槽连接歧义问题的工具。它属于 QOverload 模板类的一部分，帮助开发者在使用 Qt 5 风格的信号与槽连接（即基于函数指针的方式）时，明确指定要连接的具体重载函数版本。
//     connect(timer, &QTimer::timeout, this, QOverload<>::of(&XVideoWidget::update));
//     timer->start(1000 / 30);
}

// 刷新显示
void XVideoWidget::paintGL()
{
    // 多线程init删除创建和访问会有冲突
    std::lock_guard<std::mutex> lk(mtx_);

    // // 读到文件末尾则重置
    // if(feof(fp)) {
    //     if(fseek(fp, 0, SEEK_SET) != 0) {
    //         qDebug() << "Failed to rewind YUV file";
    //         return;
    //     }
    // }

    // 文件读取
    // 添加读取检查
    // if(fread(datas[0], 1, width_ * height_, fp) != width_ * height_) {
    //     qDebug() << "Y data read failed";
    //     return;
    // }
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texs[0]); // 0层绑定到材质
    // 修改材质内容（复制内存内容到显存）
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width_, height_, GL_RED, GL_UNSIGNED_BYTE, datas[0]);
    // 与shader uni变量关联
    glUniform1i(unis[0],0);

    // Y
    // if(fread(datas[1], 1, width_/2 * height_/2, fp) !=  width_/2 * height_/2) {
    //     qDebug() << "U data read failed";
    //     return;
    // }
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texs[1]); // 1层绑定到材质
    // 修改材质内容（复制内存内容到显存）
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width_/2, height_/2, GL_RED, GL_UNSIGNED_BYTE, datas[1]);
    // 与shader uni变量关联
    glUniform1i(unis[1],1);

    // V
    // if(fread(datas[2], 1, width_/2 * height_/2, fp) !=  width_/2 * height_/2) {
    //     qDebug() << "V data read failed";
    //     return;
    // }
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texs[2]); // 2层绑定到材质
    // 修改材质内容（复制内存内容到显存）
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width_/2, height_/2, GL_RED, GL_UNSIGNED_BYTE, datas[2]);
    // 与shader uni变量关联
    glUniform1i(unis[2],2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    qDebug() << "paintGL()";

}
void XVideoWidget::resizeGL(int w, int h)
{

    std::lock_guard<std::mutex> lk(mtx_);

    // 设置视口
    glViewport(0, 0, w, h);

    qDebug() << "resizeGL()";
}
