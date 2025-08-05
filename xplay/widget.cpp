#include "widget.h"
#include "XVideoWidget.h"
#include <QVBoxLayout>  // 用于布局管理
#include <qlogging.h>
#include <qobject.h>
#include <qpushbutton.h>
#include <QFileDialog>
#include <QMessageBox>
#include "XDemuxThread.h"

static XDemuxThread *dt;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    resize(800, 600);

    // 创建 XVideoWidget
    videoWidget = new XVideoWidget(this);
    videoOrgWidth = videoWidget->Width();
    videoOrgHeight = videoWidget->Height();
    
    // 设置窗口基础大小策略
    setMinimumSize(400, 300);

    openFileBtn = new QPushButton("打开文件",this);
    openFileBtn->move(10, 10);
    connect(openFileBtn,&QPushButton::clicked,this,&Widget::OpenFile);

    dt = new XDemuxThread();
    dt->Start();
}

Widget::~Widget() {
    delete dt;
}

void Widget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateVideoGeometry();
}
// 设置视频显示位置和大小
void Widget::updateVideoGeometry()
{
    if (videoOrgWidth <= 0 || videoOrgWidth <= 0) {
        // 如果还没有设置视频尺寸，则暂时填满窗口
        videoWidget->setGeometry(0, 0, width(), height());
        return;
    }
    
    // 计算宽高比
    float videoAspect = (float)videoOrgWidth / (float)videoOrgHeight;
    float widgetAspect = (float)width() / (float)height();
    
    int renderWidth, renderHeight, renderX, renderY;
    
    // 根据宽高比确定渲染区域
    if (widgetAspect > videoAspect) {
        // 窗口更宽，上下留空
        renderWidth = height() * videoAspect;
        renderHeight = height();
        renderX = (width() - renderWidth) / 2;
        renderY = 0;
    } else {
        // 窗口更高，左右留空
        renderWidth = width();
        renderHeight = width() / videoAspect;
        renderX = 0;
        renderY = (height() - renderHeight) / 2;
    }
    
    // 设置videoWidget的几何位置和大小
    videoWidget->setGeometry(renderX, renderY, renderWidth, renderHeight);
}

void Widget::OpenFile()
{
    // 选择文件
    QString url = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("选择文件"), "D:/", "*.mp4;;*.flv;;*.avi;;*.mkv;;*.ts");
    qDebug() << "OpenFile:" << url;
    if(url.isEmpty()) return;
    this->setWindowTitle(url);
    if(!dt->Open(url.toLocal8Bit(), videoWidget)){
        QMessageBox::information(this, "提示", "打开失败");
        return;
    }
    videoOrgWidth = videoWidget->Width();
    videoOrgHeight = videoWidget->Height();
    updateVideoGeometry();
}