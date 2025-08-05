#include "widget.h"
#include "XVideoWidget.h"
#include <QVBoxLayout>  // 用于布局管理
#include <iostream>
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

    // 添加暂停按钮
    playPauseBtn = new QPushButton("暂停", this);
    playPauseBtn->move(100, 10);
    playPauseBtn->setEnabled(false);  // 初始时不可用
    connect(playPauseBtn, &QPushButton::clicked, this, &Widget::PlayOrPause);

    // 创建滑动条作为进度条
    slider = new QSlider(Qt::Horizontal, this);
    slider->setRange(0, 1000);
    slider->setValue(0);
    connect(slider, &QSlider::sliderPressed, this, &Widget::SliderPressed);
    connect(slider, &QSlider::sliderReleased, this, &Widget::SliderReleased);
    connect(slider, &QSlider::valueChanged, this, &Widget::SliderMoved);

    dt = new XDemuxThread();
    dt->Start();

    startTimer(40);
}

Widget::~Widget() {
    delete dt;
}

void Widget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateVideoGeometry();

    // 设置进度条位置在窗口底部，宽度占满
    if (slider) {
        slider->setGeometry(0, height() - 20, width(), 30);
    }
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
    if(!dt->Open(url.toUtf8().constData(), videoWidget)){
        QMessageBox::information(this, "提示", "打开失败");
        return;
    }
    videoOrgWidth = videoWidget->Width();
    videoOrgHeight = videoWidget->Height();
    updateVideoGeometry();
    
    // 文件打开成功后启用暂停按钮
    playPauseBtn->setEnabled(true);
}


void Widget::SliderPressed()
{
    isSliderPressed = true;
}

void Widget::SliderReleased()
{
    isSliderPressed = false;
    // 当用户释放滑块时，跳转到相应位置
    // dt->Seek(slider->value());
}

void Widget::SliderMoved(int value)
{
    // 当滑块移动时更新显示，但不跳转位置（除非正在拖动）
    if (isSliderPressed) {
        // 可以在这里添加拖动时的预览功能
    }
    std::cout << "SliderMoved:" << value << std::endl;
}

void Widget::timerEvent(QTimerEvent *e){
    long long total = dt->totalMs_;
    if(total > 0){
        double pos = (double)dt->pts_ / (double)total;
        // 确保进度条值在合理范围内，当播放结束时能到达末尾
        int sliderValue = pos * 1000;
        if (sliderValue > 1000) sliderValue = 1000;
        slider->setValue(sliderValue);
    }
}

// 双击全屏
void Widget::mouseDoubleClicked(QMouseEvent *event)
{
    if(isFullScreen())
        this->showNormal();
    else
        this->showFullScreen(); 
}

void Widget::mouseClicked(QMouseEvent *event)
{
    
}

// 暂停/播放按钮处理函数
void Widget::PlayOrPause()
{
    if (!dt) return;
    
    bool isPaused = dt->IsPaused();
    dt->SetPause(!isPaused);
    
    // 更新按钮文本
    if (isPaused) {
        playPauseBtn->setText("暂停");
    } else {
        playPauseBtn->setText("播放");
    }
}