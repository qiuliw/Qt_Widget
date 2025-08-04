#include "XAudioThread.h"
#include "XDecode.h"
#include "XDemux.h"
#include "XDemuxThread.h"
#include "XVideoThread.h"
#include "XVideoWidget.h"
#include "widget.h"

#include <QApplication>
#include <iostream>
#include <qdebug.h>
#include <QThread>
#include <qlogging.h>
#include "XResample.h"
#include "XAudioPlay.h"

extern "C"{
    #include <libavutil/frame.h>
    #include <libavcodec/packet.h>
}

class TestThread : public QThread{
public:
    TestThread(QObject *parent = nullptr) : QThread(parent){}

    void Init(){
        // char *url = "hm.mp4";
        // // 解封装器初始化
        // if (!demux.Open(url)){
        //     qDebug() << "打开文件失败:" << url;
        //     return;
        // }
        // qDebug() << "打开文件成功:" << url;


        // // 音频重采样初始化
        // resample.Open(demux.CopyAPara());
        
        // // 音频播放器初始化
        // ap = XAudioPlay::Get();

        // // 获取视频帧率并计算帧间隔
        // video_fps_ = demux.GetVideoFPS();
        // if (video_fps_ <= 0) {
        //     video_fps_ = 25.0;
        //     qWarning() << "无法获取视频帧率，使用默认值25fps";
        // }
        // frame_interval_ms_ = qRound(1000.0 / video_fps_);
        // qDebug() << "视频帧率:" << video_fps_ << "，每帧间隔:" << frame_interval_ms_ << "ms";
    }

    // void run() override
    // {
    //     XAudioThread at;
    //     at.Open(demux.CopyAPara());  
    //     at.start();     
    //     XVideoThread vt;
    //     vt.Open(demux.CopyVPara(),video);
    //     vt.start();
    //     for (;;)
    //     {
    //         // 1. 读取数据包
    //         AVPacket *pkt = demux.Read();
    //         bool is_eof =! pkt; // 判断是否读到文件末尾

    //         // 2. 根据流类型分发到对应解码器
    //         XDecode *dec = nullptr;
    //         if (pkt && demux.isVideo(pkt))
    //             dec = &vdec;
    //         else if (pkt)
    //             dec = &adec;

    //         if (dec == &vdec) {
    //             vt.Push(pkt);
    //         }
    //         if(dec == &adec){
    //             at.Push(pkt); // 阻塞，音频播放线程的阻塞可能连带主线程阻塞
    //         }
            

    //         // // 3. 文件末尾时，向解码器发送空包触发flush
    //         // if (is_eof) {
    //         //     vdec.Send(nullptr); // 视频解码器flush
    //         //     adec.Send(nullptr); // 音频解码器flush
    //         // }

    //         // 4. 处理视频帧（一次send，多次recv直到无帧）
                

    //         // 5. 处理音频帧（一次send，多次recv直到无帧）
    //         // if (dec == &adec || is_eof) {
    //         //     // while (true) {
    //         //     //     AVFrame *frame = adec.Recv();
    //         //     //     if (!frame) break; // 无更多帧，退出循环
                    
    //         //     //     int re = resample.Resample(frame, pcm);
    //         //     //     if (re > 0 && ap->GetFree() >= re) {
    //         //     //         ap->Play(pcm, re);
    //         //     //     }
    //         //     //     av_frame_free(&frame);
    //         //     // }
    //         // }

    //         // 6. 释放数据包（修复崩溃：在所有解码完成后再释放）
    //         // if (pkt) {
    //         //     av_packet_free(&pkt);
    //         // }

    //         // 7. 文件末尾，退出主循环
    //         if (is_eof) {
    //             break;
    //             at.isExit_ = true;
    //         }
    //     }

    //     // 释放资源
    //     qDebug() << "播放完成";
    // }

    // // 成员变量
    // double video_fps_;       // 视频帧率
    // int frame_interval_ms_;  // 帧间隔毫秒数
    // XDemux demux;            // 解封装器
    // XDecode adec;            // 音频解码器
    // XDecode vdec;            // 视频解码器
    // XResample resample;      // 音频重采样
    // XAudioPlay *ap = nullptr;// 音频播放器
    // XVideoWidget *video = nullptr;
};

int main(int argc, char *argv[])
{
    av_log_set_level(AV_LOG_ERROR); // 设置FFmpeg日志级别

    QApplication a(argc, argv);
    Widget w;
    w.show();

    // TestThread tt;
    // tt.Init();
    // w.videoWidget->Init(tt.demux.width_, tt.demux.height_);
    // tt.video = w.videoWidget;
    // tt.start();

    XDemuxThread dt;
    dt.Open("hm.mp4", w.videoWidget);
    dt.Start();
    return a.exec();
}